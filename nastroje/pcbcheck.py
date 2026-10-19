import os
"""Kontrola DPS z KiCadu: raster mědi, propojení, mezery, shoda se schématem."""
import sys, math, re, json
import numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sx import parse, unq, find, first

RES = 40  # px/mm


def num(v): return float(v)


def fp_pads(fp):
    at = first(fp, 'at'); X, Y = num(at[1]), num(at[2]); R = num(at[3]) if len(at) > 3 else 0.0
    ref = [unq(p[2]) for p in find(fp, 'property') if unq(p[1]) == 'Reference'][0]
    side = unq(first(fp, 'layer')[1])
    out = []
    for p in find(fp, 'pad'):
        a = first(p, 'at'); lx, ly = num(a[1]), num(a[2]); pa = num(a[3]) if len(a) > 3 else 0.0
        r = math.radians(R)
        x = X + lx * math.cos(r) + ly * math.sin(r); y = Y - lx * math.sin(r) + ly * math.cos(r)
        sz = first(p, 'size'); w, h = num(sz[1]), num(sz[2])
        layers = [unq(l) for l in first(p, 'layers')[1:]]
        cu = set()
        for l in layers:
            if l == '*.Cu': cu |= {'F.Cu', 'B.Cu'}
            elif l in ('F.Cu', 'B.Cu'): cu.add(l)
        net = first(p, 'net'); net = unq(net[-1]) if net else ''
        rr = first(p, 'roundrect_rratio'); rr = num(rr[1]) if rr else 0
        drill = first(p, 'drill'); dr = num(drill[1]) if drill and len(drill) > 1 and drill[1] != 'oval' else (num(drill[2]) if drill and len(drill) > 2 else 0)
        out.append(dict(ref=ref, num=unq(p[1]), type=p[2], shape=p[3], x=x, y=y, w=w, h=h, ang=pa, rr=rr, cu=cu, net=net, drill=dr, side=side))
    return ref, out


def load(path):
    t = parse(open(path, encoding='utf-8').read())[0]
    pads = []; refs = {}
    for fp in find(t, 'footprint'):
        ref, ps = fp_pads(fp)
        props = {unq(p[1]): unq(p[2]) for p in find(fp, 'property')}
        refs[ref] = (props.get('Value'), unq(fp[1]).split(':')[-1], unq(first(fp, 'layer')[1]))
        pads += ps
    segs = [dict(l=unq(first(s, 'layer')[1]), a=(num(first(s, 'start')[1]), num(first(s, 'start')[2])), b=(num(first(s, 'end')[1]), num(first(s, 'end')[2])), w=num(first(s, 'width')[1]), net=unq(first(s, 'net')[-1])) for s in find(t, 'segment')]
    vias = [dict(x=num(first(v, 'at')[1]), y=num(first(v, 'at')[2]), d=num(first(v, 'size')[1]), drill=num(first(v, 'drill')[1]), net=unq(first(v, 'net')[-1])) for v in find(t, 'via')]
    zones = []
    for z in find(t, 'zone'):
        net = unq(first(z, 'net')[-1]) if first(z, 'net') else ''
        for fpoly in find(z, 'filled_polygon'):
            zones.append(dict(net=net, l=unq(first(fpoly, 'layer')[1]), pts=[(num(p[1]), num(p[2])) for p in find(first(fpoly, 'pts'), 'xy')]))
    edge = []
    for n in t:
        if isinstance(n, list) and n and n[0].startswith('gr_') and first(n, 'layer') and unq(first(n, 'layer')[1]) == 'Edge.Cuts':
            g = {k[0]: (num(k[1]), num(k[2])) for k in n if isinstance(k, list) and k[0] in ('start', 'end', 'mid', 'center')}
            edge.append((n[0], g))
    return dict(t=t, pads=pads, segs=segs, vias=vias, zones=zones, edge=edge, refs=refs)


def edge_pts(edge, step=0.05):
    pts = []
    for kind, g in edge:
        if kind == 'gr_line':
            a, b = g['start'], g['end']; n = max(2, int(math.dist(a, b) / step))
            pts += [(a[0] + (b[0] - a[0]) * i / n, a[1] + (b[1] - a[1]) * i / n) for i in range(n + 1)]
        elif kind == 'gr_arc':
            a, m, b = g['start'], g['mid'], g['end']
            ax, ay = a; bx, by = m; cx, cy = b
            d = 2 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by))
            ux = ((ax*ax+ay*ay)*(by-cy)+(bx*bx+by*by)*(cy-ay)+(cx*cx+cy*cy)*(ay-by))/d
            uy = ((ax*ax+ay*ay)*(cx-bx)+(bx*bx+by*by)*(ax-cx)+(cx*cx+cy*cy)*(bx-ax))/d
            r = math.hypot(ax-ux, ay-uy)
            a0 = math.atan2(ay-uy, ax-ux); a1 = math.atan2(by-uy, bx-ux); a2 = math.atan2(cy-uy, cx-ux)
            def norm(x): return (x - a0) % (2*math.pi)
            sweep = norm(a2); mid = norm(a1)
            if mid > sweep: sweep = sweep - 2*math.pi
            n = max(4, int(abs(sweep) * r / step))
            pts += [(ux + r*math.cos(a0 + sweep*i/n), uy + r*math.sin(a0 + sweep*i/n)) for i in range(n+1)]
        elif kind == 'gr_circle':
            c, e = g['center'], g['end']; r = math.dist(c, e); n = max(16, int(2*math.pi*r/step))
            pts += [(c[0] + r*math.cos(2*math.pi*i/n), c[1] + r*math.sin(2*math.pi*i/n)) for i in range(n+1)]
    return pts


class Board:
    def __init__(s, path, margin=2.0):
        s.d = load(path)
        ep = edge_pts([e for e in s.d['edge'] if e[0] != 'gr_circle'])
        xs = [p[0] for p in ep]; ys = [p[1] for p in ep]
        s.x0, s.y0 = min(xs) - margin, min(ys) - margin
        s.W = int((max(xs) - s.x0 + margin) * RES) + 1; s.H = int((max(ys) - s.y0 + margin) * RES) + 1
        nets = sorted({p['net'] for p in s.d['pads'] if p['net']} | {g['net'] for g in s.d['segs']} | {v['net'] for v in s.d['vias']} | {z['net'] for z in s.d['zones']})
        s.nets = [n for n in nets if n]
        s.nid = {n: i + 1 for i, n in enumerate(s.nets)}
        # obrys desky (maska uvnitř)
        img = Image.new('L', (s.W, s.H), 0); dr = ImageDraw.Draw(img)
        for kind, g in s.d['edge']:
            if kind == 'gr_circle': continue
            q = edge_pts([(kind, g)], 0.02)
            dr.line([s.px(p) for p in q], fill=1, width=3)
        line = np.array(img, bool)
        s.inside = ndimage.binary_fill_holes(line) & ~line | line
        s.edge_all = s.d['edge']

    def px(s, p): return ((p[0] - s.x0) * RES, (p[1] - s.y0) * RES)

    def grid(s, x0, y0, x1, y1):
        i0 = max(0, int((x0 - s.x0) * RES) - 1); i1 = min(s.W, int((x1 - s.x0) * RES) + 2)
        j0 = max(0, int((y0 - s.y0) * RES) - 1); j1 = min(s.H, int((y1 - s.y0) * RES) + 2)
        xs = s.x0 + (np.arange(i0, i1) + 0.5) / RES; ys = s.y0 + (np.arange(j0, j1) + 0.5) / RES
        X, Y = np.meshgrid(xs, ys)
        return (slice(j0, j1), slice(i0, i1)), X, Y

    def seg_mask(s, a, b, r, grow=0.0):
        r += grow
        sl, X, Y = s.grid(min(a[0], b[0]) - r, min(a[1], b[1]) - r, max(a[0], b[0]) + r, max(a[1], b[1]) + r)
        dx, dy = b[0] - a[0], b[1] - a[1]; L2 = dx*dx + dy*dy or 1e-12
        t = np.clip(((X - a[0])*dx + (Y - a[1])*dy) / L2, 0, 1)
        return sl, np.hypot(X - a[0] - t*dx, Y - a[1] - t*dy) <= r

    def pad_mask(s, p, grow=0.0):
        hw, hh = p['w']/2 + grow, p['h']/2 + grow; rad = max(hw, hh) * 1.5
        sl, X, Y = s.grid(p['x'] - rad, p['y'] - rad, p['x'] + rad, p['y'] + rad)
        a = math.radians(p['ang'])
        u = (X - p['x'])*math.cos(a) - (Y - p['y'])*math.sin(a)
        v = (X - p['x'])*math.sin(a) + (Y - p['y'])*math.cos(a)
        if p['shape'] == 'circle':
            return sl, np.hypot(u, v) <= hw
        if p['shape'] == 'oval':
            cr = min(hw, hh)
        elif p['shape'] == 'roundrect':
            cr = min(p['w'], p['h']) * p['rr'] + grow
        else:
            cr = grow
        qx = np.abs(u) - (hw - cr); qy = np.abs(v) - (hh - cr)
        return sl, np.hypot(np.maximum(qx, 0), np.maximum(qy, 0)) + np.minimum(np.maximum(qx, qy), 0) <= cr

    def copper(s, layer, grow=0.0):
        """label image: net id (0 = nic, -1 = konflikt dvou sítí)"""
        L = np.zeros((s.H, s.W), np.int32)
        def put(sl, m, k):
            sub = L[sl]
            conflict = m & (sub != 0) & (sub != k)
            sub[m & (sub == 0)] = k
            sub[conflict] = -1
        for z in s.d['zones']:
            if z['l'] != layer: continue
            img = Image.new('L', (s.W, s.H), 0); ImageDraw.Draw(img).polygon([s.px(p) for p in z['pts']], fill=1)
            put((slice(None), slice(None)), np.array(img, bool), s.nid.get(z['net'], 0) or 99999)
        for g in s.d['segs']:
            if g['l'] != layer: continue
            sl, m = s.seg_mask(g['a'], g['b'], g['w']/2, grow); put(sl, m, s.nid.get(g['net'], 99999))
        for v in s.d['vias']:
            sl, m = s.seg_mask((v['x'], v['y']), (v['x'], v['y']), v['d']/2, grow); put(sl, m, s.nid.get(v['net'], 99999))
        for p in s.d['pads']:
            if layer not in p['cu'] or p['type'] == 'np_thru_hole': continue
            sl, m = s.pad_mask(p, grow); put(sl, m, s.nid.get(p['net'], 0) or 100000 + len(p['ref']))
        return L

    def copper_nets_at(s, layer, x, y, r=0.05):
        out = set()
        for z in s.d['zones']:
            if z['l'] == layer:
                from matplotlib.path import Path
                if Path(z['pts']).contains_point((x, y)): out.add('zóna ' + z['net'])
        for g in s.d['segs']:
            if g['l'] != layer: continue
            a, b = g['a'], g['b']; dx, dy = b[0]-a[0], b[1]-a[1]; L2 = dx*dx+dy*dy or 1e-12
            t = max(0, min(1, ((x-a[0])*dx+(y-a[1])*dy)/L2))
            if math.hypot(a[0]+t*dx-x, a[1]+t*dy-y) <= g['w']/2 + r: out.add('cesta ' + g['net'])
        for p in s.d['pads']:
            if layer in p['cu'] and p['type'] != 'np_thru_hole' and abs(x-p['x']) <= p['w']/2+r+0.3 and abs(y-p['y']) <= p['h']/2+r+0.3: out.add(f"pad {p['ref']}.{p['num']} {p['net']}")
        for v in s.d['vias']:
            if math.hypot(x-v['x'], y-v['y']) <= v['d']/2 + r: out.add('via ' + v['net'])
        return sorted(out)

    def check(s, clr=0.2):
        res = dict(shorts=[], clearance=[], opens=[], islands=0)
        layers = sorted({g['l'] for g in s.d['segs']} | {l for p in s.d['pads'] for l in p['cu']} | {z['l'] for z in s.d['zones']})
        comp = {}; labels = {}
        for layer in layers:
            L = s.copper(layer)
            if (L == -1).any():
                lab, n = ndimage.label(L == -1)
                for k in range(1, n + 1):
                    ys, xs = np.nonzero(lab == k)
                    j, i = int(ys.mean()), int(xs.mean())
                    win = s.copper_nets_at(layer, xs.mean()/RES + s.x0, ys.mean()/RES + s.y0)
                    res['shorts'].append((layer, int(len(xs)), round(xs.mean()/RES + s.x0, 2), round(ys.mean()/RES + s.y0, 2), win))
            # mezery: každá síť nafouknutá o clr/2 nesmí zasáhnout jinou síť nafouknutou o clr/2
            G = s.copper(layer, grow=clr/2 - 0.5/RES)
            if (G == -1).any():
                lab, n = ndimage.label(G == -1)
                for k in range(1, n + 1):
                    ys, xs = np.nonzero(lab == k)
                    cx, cy = xs.mean()/RES + s.x0, ys.mean()/RES + s.y0
                    # které sítě
                    near = set()
                    j, i = int(ys.mean()), int(xs.mean())
                    win = L[max(0, j-int(clr*RES)-4):j+int(clr*RES)+5, max(0, i-int(clr*RES)-4):i+int(clr*RES)+5]
                    near = {s.nets[v-1] if 0 < v <= len(s.nets) else str(v) for v in np.unique(win) if v > 0}
                    res['clearance'].append((layer, round(cx, 2), round(cy, 2), sorted(near)))
            # komponenty mědi po sítích
            for k in np.unique(L):
                if k <= 0: continue
                lab, n = ndimage.label(L == k)
                labels[(layer, int(k))] = lab
        # sjednocení přes vrstvy (THT pady, via)
        par = {}
        def f(a):
            par.setdefault(a, a)
            while par[a] != a: par[a] = par[par[a]]; a = par[a]
            return a
        def u(a, b): par[f(a)] = f(b)
        def node(layer, x, y, net):
            k = s.nid.get(net, 0)
            lab = labels.get((layer, k))
            if lab is None: return None
            i, j = int((x - s.x0)*RES), int((y - s.y0)*RES)
            c = int(lab[j, i])
            return (layer, k, c) if c else None
        for v in s.d['vias']:
            ns = [node(l, v['x'], v['y'], v['net']) for l in ('F.Cu', 'B.Cu')]
            ns = [n for n in ns if n]
            for a in ns[1:]: u(ns[0], a)
        padnodes = []
        for p in s.d['pads']:
            if not p['net'] or p['net'].startswith('unconnected'): continue
            ns = [node(l, p['x'], p['y'], p['net']) for l in sorted(p['cu'])]
            ns = [n for n in ns if n]
            if len(ns) > 1 and p['type'] == 'thru_hole':
                for a in ns[1:]: u(ns[0], a)
            padnodes.append((p, ns))
        bynet = {}
        for p, ns in padnodes:
            if not ns: continue
            # SMD na jedné vrstvě: jeden uzel; THT: sjednocené
            bynet.setdefault(p['net'], {}).setdefault(f(ns[0]), []).append(f"{p['ref']}.{p['num']}")
        for net, groups in bynet.items():
            if len(groups) > 1:
                res['opens'].append((net, [sorted(g)[:6] for g in groups.values()]))
        res['tht_one_side'] = []  # THT pady s cestou jen na horní straně (u neprokovené DPS nutno pájet shora)
        for p, ns in padnodes:
            if p['type'] != 'thru_hole': continue
            top = any(g['l'] == 'F.Cu' and (math.dist(g['a'], (p['x'], p['y'])) < max(p['w'], p['h'])/2 + 0.05 or math.dist(g['b'], (p['x'], p['y'])) < max(p['w'], p['h'])/2 + 0.05) for g in s.d['segs'])
            if top: res['tht_one_side'].append(f"{p['ref']}.{p['num']}")
        return res


def sch_nets(sch):
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import sch_netlist as check
    nets, unc, probs, refs, _, _ = check.analyze(sch)
    return nets, unc, probs, refs


def compare_sch(board, sch):
    nets, unc, probs, refs = sch_nets(sch)
    out = []
    padnet = {f"{p['ref']}.{p['num']}": p['net'] for p in board.d['pads']}
    pcbrefs = set(board.d['refs']); schrefs = {r for r in refs if not r.startswith('#')}
    if pcbrefs - schrefs: out.append(('jen v DPS', sorted(pcbrefs - schrefs)))
    if schrefs - pcbrefs: out.append(('jen ve schématu', sorted(schrefs - pcbrefs)))
    for r in sorted(pcbrefs & schrefs):
        sv, sf = refs[r][1], (refs[r][2] or '').split(':')[-1]
        pv, pf, side = board.d['refs'][r]
        if sv != pv or (sf and sf != pf): out.append(('hodnota/pouzdro', r, (sv, sf), (pv, pf)))
    for name, members in nets.items():
        pn = {padnet.get(m, '??') for m in members if m.split('.')[0] in pcbrefs}
        if len(pn) > 1: out.append(('síť rozdělena v DPS', name, sorted(pn)))
    # dvě schématické sítě sloučené v DPS
    seen = {}
    for name, members in nets.items():
        for m in members:
            n = padnet.get(m)
            if n: seen.setdefault(n, set()).add(name)
    for n, names in seen.items():
        if len(names) > 1: out.append(('sítě sloučené v DPS', n, sorted(names)))
    return out, unc, probs


