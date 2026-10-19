"""Knoflík pro EC11 – nízký, zakrývá osu. Generuje STL (bez OpenSCADu)."""
import math, struct, sys
N = 360
def knob(shaft, dtype="D", R=12.5, GAP=1.0, FLUTES=30, FD=0.55, DIMPLE=True):
    BORE_R = 3.05 if dtype == "D" else 2.95        # D: Ø6,1 + plocha ; vroubek: Ø5,9 nalisovat
    FLAT   = 1.55                                   # D osa 6 mm s plochou 4,5 mm
    zb   = shaft - GAP + 0.3                        # strop díry (0,3 nad koncem osy)
    H    = zb + 1.2                                 # horní stěna nad osou 1,2 mm
    RT   = 1.2                                      # zaoblení horní hrany
    Ri, Rb, hz = R - FD - 1.8, 5.5, H - 3.1         # vnitřní stěna sukně, náboj, strop dutiny
    th = [2*math.pi*i/N for i in range(N)]
    def flute(t, fade): return FD*fade*(0.5-0.5*math.cos(FLUTES*t))**2
    def ring(rf, zf): return [(rf(t)*math.cos(t), rf(t)*math.sin(t), zf(t)) for t in th]
    def top(rho, t):
        z = H
        if DIMPLE:                                  # důlek pro rychlé točení prstem
            dx, dy = rho*math.cos(t) - 7.0, rho*math.sin(t); d2 = (dx*dx+dy*dy)/3.8**2
            if d2 < 1: z -= 1.0*(1-d2)**1.5
        return z
    rings = [("pt", (0, 0, H))]
    # horní plocha od středu ven
    k = int((R-RT)/0.2)
    for i in range(1, k+1):
        rho = (R-RT)*i/k; rings.append(("r", ring(lambda t, r=rho: r, lambda t, r=rho: top(r, t))))
    # zaoblení horní hrany
    for i in range(1, 9):
        a = math.pi/2*(1 - i/8); fade = i/8
        rings.append(("r", ring(lambda t, a=a, f=fade: R-RT+RT*math.cos(a)-flute(t, f), lambda t, a=a: H-RT+RT*math.sin(a))))
    # bok s drážkami
    zs = [H-RT-(H-RT-0.6)*i/12 for i in range(1, 13)]
    for z in zs: rings.append(("r", ring(lambda t: R-flute(t, 1.0), lambda t, z=z: z)))
    # sražení spodní hrany 0,5
    rings.append(("r", ring(lambda t: R-0.5-flute(t, 0.3), lambda t: 0.0)))
    # spodek -> vnitřní stěna sukně -> strop dutiny -> náboj -> díra
    rings.append(("r", ring(lambda t: Ri, lambda t: 0.0)))
    rings.append(("r", ring(lambda t: Ri, lambda t: hz)))
    rings.append(("r", ring(lambda t: Rb, lambda t: hz)))
    rings.append(("r", ring(lambda t: Rb, lambda t: 0.0)))
    def rbore(t, extra=0.0):
        r = BORE_R + extra
        if dtype == "D" and math.cos(t) > 0: r = min(r, (FLAT+extra)/math.cos(t))
        return r
    rings.append(("r", ring(lambda t: rbore(t, 0.5), lambda t: 0.0)))      # sražení vstupu díry
    rings.append(("r", ring(lambda t: rbore(t), lambda t: 0.5)))
    rings.append(("r", ring(lambda t: rbore(t), lambda t: zb)))
    rings.append(("pt", (0, 0, zb)))
    T = []
    for (ka, A), (kb, B) in zip(rings, rings[1:]):
        for i in range(N):
            j = (i+1) % N
            if ka == "pt":   T.append((A, B[j], B[i]))
            elif kb == "pt": T.append((A[i], A[j], B))
            else:            T += [(A[i], A[j], B[j]), (A[i], B[j], B[i])]
    vol = sum((a[0]*(b[1]*c[2]-b[2]*c[1])-a[1]*(b[0]*c[2]-b[2]*c[0])+a[2]*(b[0]*c[1]-b[1]*c[0]))/6 for a, b, c in T)
    if vol < 0: T = [(a, c, b) for a, b, c in T]; vol = -vol
    return T, H, vol
def stl(fn, T):
    with open(fn, 'wb') as f:
        f.write(b'CNC pendant knob'.ljust(80, b' ')); f.write(struct.pack('<I', len(T)))
        for a, b, c in T:
            u = [b[k]-a[k] for k in range(3)]; v = [c[k]-a[k] for k in range(3)]
            n = [u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]]; l = math.sqrt(sum(x*x for x in n)) or 1
            f.write(struct.pack('<12fH', *[x/l for x in n], *a, *b, *c, 0))
if __name__ == "__main__":
    import os; os.chdir(sys.argv[1])
    for dt, nm in (("D", "D_osa"), ("K", "vroubkovana_osa")):
        for s in (9, 10, 11, 12, 13, 14):
            T, H, vol = knob(s, dt)
            stl(f"knoflik_{nm}_{s}mm_nad_panelem.stl", T)
            print(nm, s, "vyska knofliku %.1f mm, nad panelem %.1f mm, objem %.0f mm3, %d tri" % (H, H+1.0, vol, len(T)))
