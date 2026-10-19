"""Kompletní kontrola DPS v repozitáři: schéma ↔ DPS, DRC, gerbery, simulace izolace, vrtání, ořezy.

Použití (z kořene repozitáře):  python nastroje/kontrola.py
Potřebuje: Python 3.10+, numpy, scipy, pillow, matplotlib  (pip install numpy scipy pillow matplotlib)

Po úpravě desky v KiCadu: vyexportuj gerbery, vygeneruj G-kódy (FlatCAM) se stejnými názvy jako
v 02_DPS_frezovani a spusť tuto kontrolu. Nové náhledy desek: python nastroje/render.py
"""
import os, sys, re, math
import numpy as np
from scipy import ndimage
HERE = os.path.dirname(os.path.abspath(__file__)); R = os.path.join(HERE, '..')
sys.path.insert(0, HERE)
import pcbcheck as P, isosim as S

KI = lambda b: f'{R}/01_DPS_KiCad/{b}/{b}.kicad_pcb'
SCH = lambda b: f'{R}/01_DPS_KiCad/{b}/{b}.kicad_sch'
F = lambda *p: os.path.join(R, '02_DPS_frezovani', *p)

# mapování souřadnic G-kódu (nula stroje) -> souřadnice KiCadu
MAP_A_TOP = lambda gx, gy: (gx + 44, 102 - gy)       # nula = střed levé dolní díry
MAP_A_BOT = lambda gx, gy: (gx + 44, gy + 44)        # spodní strana, zrcadleno přes vodorovnou osu
MAP_B = lambda gx, gy: (gx + 100, gy + 100)          # roh desky, zrcadleno
MAP_C = lambda gx, gy: (gx + 100, 100 - gy)          # roh desky

ok = True
def bad(msg):
    global ok; ok = False; print('   CHYBA:', msg)


def gerber(fn):
    s = open(fn).read(); m = re.search(r'%FSLAX(\d)(\d)Y', s); dec = int(m[2])
    ap = {int(a[1]): a[2] for a in re.finditer(r'%ADD(\d+)([A-Za-z_]+)', s)}
    cur = None; x = y = 0; draws = []; flashes = []; region = False
    for blk in re.findall(r'%[^%]*%|[^*%]+\*', s):
        blk = blk.strip().rstrip('*').strip()
        if blk.startswith('%') or not blk: continue
        if blk.startswith('G36'): region = True; continue
        if blk.startswith('G37'): region = False; continue
        md = re.match(r'^(?:G0?1)?D(\d+)$', blk)
        if md and int(md[1]) >= 10: cur = int(md[1]); continue
        mm = re.match(r'^(?:G0?[123])?(?:X(-?\d+))?(?:Y(-?\d+))?(?:I-?\d+)?(?:J-?\d+)?D0?([123])$', blk)
        if mm:
            nx = int(mm[1]) / 10**dec if mm[1] else x; ny = int(mm[2]) / 10**dec if mm[2] else y
            if mm[3] == '1' and not region and ap.get(cur) == 'C': draws.append(((x, -y), (nx, -ny)))
            if mm[3] == '3': flashes.append((nx, -ny))
            x, y = nx, ny
    return draws, flashes


def plunges(fn):
    pts = set(); x = y = None
    for l in open(fn, errors='ignore'):
        m = re.search(r'X(-?[\d.]+)', l); n = re.search(r'Y(-?[\d.]+)', l)
        if m: x = float(m[1])
        if n: y = float(n[1])
        if 'Z-' in l and x is not None: pts.add((round(x, 2), round(y, 2)))
    return pts


boards = {'A': 'pendant_A_horni', 'B': 'pendant_B_spodni', 'C': 'pendant_C_led'}
for k, b in boards.items():
    print(f'== Deska {k} ({b})')
    B = P.Board(KI(b)); r = B.check(0.2)
    cmp_, unc, probs = P.compare_sch(B, SCH(b))
    print(f"   sítí {len(B.nets)}, cest {len(B.d['segs'])}, via {len(B.d['vias'])}")
    if r['shorts']: bad(f"zkraty {r['shorts']}")
    if r['clearance']: bad(f"mezery < 0,2 mm: {r['clearance'][:5]}")
    if r['opens']: bad(f"nepropojené sítě {r['opens']}")
    if cmp_: bad(f"schéma vs DPS {cmp_[:5]}")
    if unc: bad(f"nepřipojené piny ve schématu {unc}")
    if k == 'A': print('   pájet i shora:', ', '.join(r['tht_one_side']))

print('== Gerbery')
for k, L, g in (('A', 'F.Cu', 'A_horni/gerber/pendant_A_horni-F_Cu.gbr'), ('A', 'B.Cu', 'A_horni/gerber/pendant_A_horni-B_Cu.gbr'),
                ('B', 'B.Cu', 'B_spodni/gerber/pendant_B_spodni-B_Cu.gbr'), ('C', 'F.Cu', 'C_led/gerber/pendant_C_led-F_Cu.gbr')):
    B = P.Board(KI(boards[k])); dr, fl = gerber(F(g))
    key = lambda a, b: tuple(sorted([(round(a[0], 2), round(a[1], 2)), (round(b[0], 2), round(b[1], 2))]))
    ps = {key(s_['a'], s_['b']) for s_ in B.d['segs'] if s_['l'] == L}; gs = {key(a, b) for a, b in dr}
    pads = [(p['x'], p['y']) for p in B.d['pads'] if L in p['cu'] and p['type'] != 'np_thru_hole'] + [(v['x'], v['y']) for v in B.d['vias']]
    miss = [p for p in pads if min(math.dist(p, f) for f in fl) > 0.02]
    print(f'   {k} {L}: cesty {len(ps)}/{len(gs)}, pady bez gerberu {len(miss)}')
    if ps != gs or miss: bad(f'gerber {g} neodpovídá DPS – vyexportuj znovu')

print('== Izolace (simulace)')
def iso(b, fn, mf, L):
    return S.simulate(KI(b), fn, mf, label=L)
T = iso(boards['A'], F('A_horni', 'A2_horni_med_FCUV01_bez_der_freza0.18.nc'), MAP_A_TOP, 'F.Cu')
Bt = iso(boards['A'], F('A_horni', 'A1_spodni_med_ZRCADLENO_freza0.16.nc'), MAP_A_BOT, 'B.Cu')
BA = T['B']; RES = P.RES
labs = {L: ndimage.label(BA.inside & ~r_['cut'])[0] for L, r_ in (('F.Cu', T), ('B.Cu', Bt))}
par = {}
def f_(a):
    par.setdefault(a, a)
    while par[a] != a: par[a] = par[par[a]]; a = par[a]
    return a
def node(L, x, y):
    c = labs[L][int((y - BA.y0) * RES), int((x - BA.x0) * RES)]; return (L, int(c)) if c else None
for x, y in [(v['x'], v['y']) for v in BA.d['vias']] + [(p['x'], p['y']) for p in BA.d['pads'] if p['type'] == 'thru_hole']:
    a, c = node('F.Cu', x, y), node('B.Cu', x, y)
    if a and c: par[f_(a)] = f_(c)
comp = {}
for p in BA.d['pads']:
    if not p['net'] or p['type'] == 'np_thru_hole': continue
    for L in p['cu']:
        n = node(L, p['x'], p['y'])
        if n: comp.setdefault(f_(n), set()).add(p['net'])
netc = {}
for c, v in comp.items():
    for n in v: netc.setdefault(n, set()).add(c)
sh = [v for v in comp.values() if len(v) > 1]; op = [n for n, v in netc.items() if len(v) > 1]
print(f'   A (A1+A2): zkraty {len(sh)}, přerušení {len(op)}')
if sh or op: bad(f'izolace A: {sh[:3]} {op[:5]}')
for k, fn, mf, L in (('B', F('B_spodni', 'B2_med_ZRCADLENO_bez_rohovych_der_freza0.18.nc'), MAP_B, 'B.Cu'),
                     ('C', F('C_led', 'C1_med_freza0.18.nc'), MAP_C, 'F.Cu')):
    r_ = iso(boards[k], fn, mf, L)
    print(f"   {k}: zkraty {len(r_['shorts'])}, přerušení {len(r_['split'])}")
    if r_['shorts'] or r_['split']: bad(f"izolace {k}: {r_['shorts'][:3]} {r_['split'][:5]}")

print('== Vrtání')
for k, files, filt, mf in (('A', [F('A_horni', 'A3_vrtani_0.4mm_prokovy.nc')], lambda d: d < 0.5, MAP_A_TOP),
                           ('A', [F('A_horni', 'A4_vrtani_1.0mm_soucastky.nc')], lambda d: 0.5 < d < 2, MAP_A_TOP),
                           ('A', [os.path.join(R, '03_pripravek_MDF', 'vrtani_DPS', 'A0_vrtani_rohovych_der_2.5mm_do_pripravku.nc')], lambda d: d > 3, MAP_A_TOP),
                           ('B', [F('B_spodni', 'B3_vrtani_1.0mm.nc')], lambda d: d < 2, MAP_B),
                           ('B', [F('B_spodni', 'B1_vrtani_rohovych_der_2.5mm_do_pripravku.nc')], lambda d: d > 3, MAP_B),
                           ('C', [F('C_led', 'C2_vrtani_1.0mm.nc')], lambda d: d > 0, MAP_C)):
    B = P.Board(KI(boards[k]))
    H = [(p['x'], p['y'], p['drill']) for p in B.d['pads'] if p['drill'] > 0] + [(v['x'], v['y'], v['drill']) for v in B.d['vias']]
    Q = set().union(*[plunges(x) for x in files])
    inv = lambda gx, gy: mf(gx, gy)
    T_ = [(x, y) for x, y, d in H if filt(d)]
    Qp = [inv(*q) for q in Q]
    miss = [t for t in T_ if min(math.dist(t, q) for q in Qp) > 0.02]
    extra = [q for q in Qp if min(math.dist(t, q) for t in T_) > 0.02]
    print(f'   {k} {os.path.basename(files[0])}: děr {len(T_)}, v programu {len(Q)}, chybí {len(miss)}, navíc {len(extra)}')
    if miss or extra: bad(f'vrtání {files[0]}')

print('\nVÝSLEDEK:', 'vše v pořádku' if ok else 'NALEZENY CHYBY')
sys.exit(0 if ok else 1)
