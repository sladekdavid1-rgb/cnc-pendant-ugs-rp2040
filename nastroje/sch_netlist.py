import os
import sys, os, json, math
sys.path.insert(0, os.path.dirname(__file__))
from sx import parse, unq, find, first

def load(path):
    t = parse(open(path, encoding="utf-8").read())[0]
    lib = {}
    for s in find(first(t, "lib_symbols"), "symbol"):
        pins = []
        for sub in find(s, "symbol"):
            for p in find(sub, "pin"):
                at = first(p, "at"); pins.append((unq(first(p, "number")[1]), float(at[1]), float(at[2])))
        lib[unq(s[1])] = dict(pins=pins, power=first(s, "power") is not None, node=s)
    return t, lib

def xf(lx, ly, x, y, a):
    r = math.radians(a)
    return (round(x + lx * math.cos(r) - ly * math.sin(r), 3), round(y - (lx * math.sin(r) + ly * math.cos(r)), 3))

def analyze(path):
    t, lib = load(path)
    par = {}
    def f(a):
        par.setdefault(a, a)
        while par[a] != a:
            par[a] = par[par[a]]; a = par[a]
        return a
    def u(a, b): par[f(a)] = f(b)
    P = lambda a: (round(float(a[1]), 3), round(float(a[2]), 3))
    wires = []
    for w in find(t, "wire"):
        xy = find(first(w, "pts"), "xy"); a, b = P(xy[0]), P(xy[1]); wires.append((a, b)); u(a, b)
    pinpts = {}; powernames = {}; refs = {}
    for s in find(t, "symbol"):
        lid = unq(first(s, "lib_id")[1]); at = first(s, "at"); x, y, a = float(at[1]), float(at[2]), float(at[3])
        props = {unq(p[1]): unq(p[2]) for p in find(s, "property")}
        ref = props["Reference"]; refs[ref] = (lid, props.get("Value"), props.get("Footprint"))
        for num, lx, ly in lib[lid]["pins"]:
            pt = xf(lx, ly, x, y, a); f(pt)
            if lib[lid]["power"]:
                if lid != "power:PWR_FLAG": powernames.setdefault(props["Value"], []).append(pt)
                else: pinpts[(ref, num)] = pt
            else:
                pinpts[(ref, num)] = pt
    labels = {}
    for l in find(t, "label"):
        pt = P(first(l, "at")); f(pt); labels.setdefault(unq(l[1]), []).append(pt)
    for d in (labels, powernames):
        for name, pts in d.items():
            for p in pts[1:]: u(pts[0], p)
    ncs = set(P(first(n, "at")) for n in find(t, "no_connect"))
    # kontroly
    problems = []
    endpoints = {}
    for a, b in wires:
        for p in (a, b): endpoints[p] = endpoints.get(p, 0) + 1
    allpins = set(pinpts.values()) | set(p for v in powernames.values() for p in v)
    allabels = set(p for v in labels.values() for p in v)
    for p, cnt in endpoints.items():
        if cnt == 1 and p not in allpins and p not in allabels:
            problems.append(f"visící konec drátu {p}")
    for a, b in wires:   # T-spoje bez rozdělení
        for p in endpoints:
            if p in (a, b): continue
            if (a[0] == b[0] == p[0] and min(a[1], b[1]) < p[1] < max(a[1], b[1])) or (a[1] == b[1] == p[1] and min(a[0], b[0]) < p[0] < max(a[0], b[0])):
                problems.append(f"bod {p} leží uprostřed drátu {a}-{b}")
        for pt in allpins:
            if (a[0] == b[0] == pt[0] and min(a[1], b[1]) < pt[1] < max(a[1], b[1])) or (a[1] == b[1] == pt[1] and min(a[0], b[0]) < pt[0] < max(a[0], b[0])):
                problems.append(f"pin {pt} uprostřed drátu")
    rootname = {}
    for d in (labels, powernames):
        for name, pts in d.items():
            r = f(pts[0])
            if r in rootname and rootname[r] != name: problems.append(f"síť má dvě jména: {rootname[r]} / {name}")
            rootname[r] = name
    nets = {}; unconnected = []
    groups = {}
    for (ref, num), pt in pinpts.items():
        groups.setdefault(f(pt), []).append((ref, num, pt))
    for r, members in groups.items():
        name = rootname.get(r)
        real = [m for m in members if not m[0].startswith("#")]
        if name is None:
            if len(members) == 1:
                ref, num, pt = members[0]
                if pt not in ncs: unconnected.append(f"{ref}.{num}")
                continue
            name = "?" + "+".join(f"{m[0]}.{m[1]}" for m in members)
        nets.setdefault(name, set()).update(f"{m[0]}.{m[1]}" for m in real)
    for name, pts in list(labels.items()):
        if len(pts) == 1 and not any(name == n for n in nets): problems.append(f"štítek {name} jen jednou")
    return nets, unconnected, problems, refs, t, lib

if __name__ == "__main__":
    out, exp = sys.argv[1], json.load(open(sys.argv[1] + "/_expected.json"))
    for proj in ("pendant_A_horni", "pendant_B_spodni", "pendant_C_led"):
        nets, unc, probs, refs, _, _ = analyze(f"{out}/{proj}/{proj}.kicad_sch")
        e = {k: set(v) for k, v in exp[proj].items()}
        diff = 0
        for n in sorted(set(nets) | set(e)):
            g = nets.get(n, set()); x = e.get(n, set())
            if (n.startswith("LEDA") or n.startswith("SWD") or n.startswith("?")) :   # nepojmenovaná síť R-LED
                continue
            if g != x:
                diff += 1; print(proj, "ROZDÍL", n, "chybí", sorted(x - g)[:6], "navíc", sorted(g - x)[:6])
        # nepojmenované sítě porovnat s LEDA
        led_un = [v for k, v in nets.items() if k.startswith("?")]
        ledA = [v for k, v in e.items() if k.startswith(("LEDA","SWD"))]
        for v in ledA:
            if v not in led_un: diff += 1; print(proj, "chybí síť", sorted(v))
        for v in led_un:
            if v not in ledA: diff += 1; print(proj, "neočekávaná síť", sorted(v))
        print(proj, f"součástek {len([r for r in refs if not r.startswith('#')])}, sítí {len(nets)}, nepřipojené piny: {unc}, problémy: {probs[:8]}, rozdílů: {diff}")
