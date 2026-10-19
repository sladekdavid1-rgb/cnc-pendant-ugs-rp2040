# CNC pendant – krabička + celá sestava pro Autodesk Fusion
# Spuštění: Utilities → ADD-INS → Scripts and Add-Ins → "+" (Create / link script) → vyber složku s tímto souborem → Run.
# Vytvoří nový design, každý díl jako samostatnou komponentu. Jednotky mm (API pracuje v cm → převod /10).
import adsk.core, adsk.fusion, traceback, json

PARTS = json.loads(r"""[{"name": "Krabicka", "color": "#3a3f47", "merge": true, "ops": [["+", [["rr", 0, 0, 101.0, 71.0, 7.0]], 0, 25.700000000000003], ["-", [["rr", 2.6, 2.6, 98.4, 68.4, 4.4]], 2.0, 25.700000000000003], ["+", [["c", 6.5, 64.5, 8.0], ["c", 94.5, 64.5, 8.0], ["c", 6.5, 6.5, 8.0], ["c", 94.5, 6.5, 8.0]], 0, 7.0], ["-", [["c", 6.5, 64.5, 3.4], ["c", 94.5, 64.5, 3.4], ["c", 6.5, 6.5, 3.4], ["c", 94.5, 6.5, 3.4]], 0, 7.0], ["-", [["c", 6.5, 64.5, 6.5], ["c", 94.5, 64.5, 6.5], ["c", 6.5, 6.5, 6.5], ["c", 94.5, 6.5, 6.5]], 0, 2.5], ["-", [["rr", -1.0, 28.39, 3.1, 41.39, 0.0]], 9.7, 17.7]]}, {"name": "Panel", "color": "#1b1b1b", "ops": [["+", [["rr", 3.0, 3.0, 98.0, 68.0, 4.0]], 24.700000000000003, 25.700000000000003], ["-", [["c", 12.0, 58.0, 4.0], ["c", 21.0, 58.0, 4.0], ["c", 30.0, 58.0, 4.0], ["c", 39.0, 58.0, 4.0], ["c", 48.0, 58.0, 4.0], ["c", 57.0, 58.0, 4.0], ["c", 12.0, 47.5, 4.0], ["c", 30.0, 47.5, 4.0], ["c", 48.0, 47.5, 4.0], ["c", 57.0, 47.5, 4.0], ["c", 12.0, 37.0, 4.0], ["c", 21.0, 37.0, 4.0], ["c", 39.0, 37.0, 4.0], ["c", 48.0, 37.0, 4.0], ["c", 12.0, 26.5, 4.0], ["c", 30.0, 26.5, 4.0], ["c", 48.0, 26.5, 4.0], ["c", 57.0, 26.5, 4.0], ["c", 21.0, 15.0, 4.0], ["c", 30.0, 15.0, 4.0], ["c", 39.0, 15.0, 4.0], ["c", 74.5, 60.5, 2.0], ["c", 78.5, 60.5, 2.0], ["c", 82.5, 60.5, 2.0], ["c", 68.5, 51.5, 2.0], ["c", 72.5, 51.5, 2.0], ["c", 76.5, 51.5, 2.0], ["c", 80.5, 51.5, 2.0], ["c", 84.5, 51.5, 2.0], ["c", 88.5, 51.5, 2.0], ["c", 72.5, 43.0, 2.0], ["c", 76.5, 43.0, 2.0], ["c", 80.5, 43.0, 2.0], ["c", 84.5, 43.0, 2.0], ["c", 68.5, 34.5, 2.0], ["c", 72.5, 34.5, 2.0], ["c", 76.5, 34.5, 2.0], ["c", 80.5, 34.5, 2.0], ["c", 84.5, 34.5, 2.0], ["c", 88.5, 34.5, 2.0], ["c", 78.5, 18.5, 7.0], ["c", 6.5, 64.5, 3.2], ["c", 94.5, 64.5, 3.2], ["c", 6.5, 6.5, 3.2], ["c", 94.5, 6.5, 3.2]], 24.700000000000003, 25.700000000000003]]}, {"name": "DPS_A", "color": "#1f6f3a", "ops": [["+", [["rr", 3.0, 3.0, 98.0, 68.0, 4.0]], 19.6, 21.200000000000003], ["-", [["c", 6.5, 64.5, 3.2], ["c", 94.5, 64.5, 3.2], ["c", 6.5, 6.5, 3.2], ["c", 94.5, 6.5, 3.2], ["c", 78.5, 18.5, 10.0]], 19.6, 21.200000000000003]]}, {"name": "DPS_B", "color": "#2d7a46", "ops": [["+", [["rr", 3.0, 3.0, 98.0, 68.0, 4.0]], 7.0, 8.6], ["-", [["c", 6.5, 64.5, 3.2], ["c", 94.5, 64.5, 3.2], ["c", 6.5, 6.5, 3.2], ["c", 94.5, 6.5, 3.2]], 7.0, 8.6]]}, {"name": "DPS_C", "color": "#2f8f52", "ops": [["+", [["poly", [[62.5, 64.0], [90.0, 64.0], [90.0, 61.0], [93.5, 61.0], [93.5, 31.0], [62.5, 31.0]]]], 22.700000000000003, 23.500000000000004]]}, {"name": "Sloupky_AB_M3x11_FF", "color": "#c8a24a", "ops": [["+", [["c", 6.5, 64.5, 5.5], ["c", 94.5, 64.5, 5.5], ["c", 6.5, 6.5, 5.5], ["c", 94.5, 6.5, 5.5]], 8.6, 19.6], ["-", [["c", 6.5, 64.5, 2.5], ["c", 94.5, 64.5, 2.5], ["c", 6.5, 6.5, 2.5], ["c", 94.5, 6.5, 2.5]], 8.6, 19.6]]}, {"name": "Distance_A_panel_3_5", "color": "#e8e8e0", "ops": [["+", [["c", 6.5, 64.5, 6.0], ["c", 94.5, 64.5, 6.0], ["c", 6.5, 6.5, 6.0], ["c", 94.5, 6.5, 6.0]], 21.200000000000003, 24.700000000000003], ["-", [["c", 6.5, 64.5, 3.2], ["c", 94.5, 64.5, 3.2], ["c", 6.5, 6.5, 3.2], ["c", 94.5, 6.5, 3.2]], 21.200000000000003, 24.700000000000003]]}, {"name": "Sroub_horni_M3x10_ISO7380", "color": "#9aa0a6", "ops": [["+", [["c", 6.5, 64.5, 5.7], ["c", 94.5, 64.5, 5.7], ["c", 6.5, 6.5, 5.7], ["c", 94.5, 6.5, 5.7]], 25.700000000000003, 27.35], ["+", [["c", 6.5, 64.5, 3.0], ["c", 94.5, 64.5, 3.0], ["c", 6.5, 6.5, 3.0], ["c", 94.5, 6.5, 3.0]], 15.700000000000003, 25.700000000000003]]}, {"name": "Sroub_spodni_M3x10", "color": "#9aa0a6", "ops": [["+", [["c", 6.5, 64.5, 5.5], ["c", 94.5, 64.5, 5.5], ["c", 6.5, 6.5, 5.5], ["c", 94.5, 6.5, 5.5]], 0.8500000000000001, 2.5], ["+", [["c", 6.5, 64.5, 3.0], ["c", 94.5, 64.5, 3.0], ["c", 6.5, 6.5, 3.0], ["c", 94.5, 6.5, 3.0]], 2.5, 12.5]]}, {"name": "Pico", "color": "#2b6cb0", "ops": [["+", [["rr", 8.629999999999999, 24.39, 59.63, 45.39, 0.5]], 11.1, 12.1]]}, {"name": "Pico_USB_C", "color": "#c0c4c8", "ops": [["+", [["rr", 7.3, 30.39, 14.6, 39.39, 0.3]], 12.1, 15.3]]}, {"name": "Pico_listy", "color": "#222222", "ops": [["+", [["rr", 8.73, 42.51, 59.53, 45.050000000000004, 0], ["rr", 8.73, 24.73, 59.53, 27.27, 0]], 8.6, 11.1]]}, {"name": "EC11E", "color": "#8a8f96", "ops": [["+", [["rr", 72.5, 11.8, 84.5, 25.2, 0]], 8.6, 15.1], ["+", [["c", 78.5, 18.5, 7.0]], 15.1, 20.1], ["+", [["c", 78.5, 18.5, 6.0]], 20.1, 38.6]]}, {"name": "Knoflik_D25", "color": "#555a60", "ops": [["+", [["c", 78.5, 18.5, 25.0]], 26.700000000000003, 41.7], ["-", [["c", 78.5, 18.5, 6.0]], 26.700000000000003, 38.7]]}, {"name": "Tlacitka_6x6x6", "color": "#2a2a2a", "ops": [["+", [["rr", 9.0, 55.0, 15.0, 61.0, 0], ["rr", 18.0, 55.0, 24.0, 61.0, 0], ["rr", 27.0, 55.0, 33.0, 61.0, 0], ["rr", 36.0, 55.0, 42.0, 61.0, 0], ["rr", 45.0, 55.0, 51.0, 61.0, 0], ["rr", 54.0, 55.0, 60.0, 61.0, 0], ["rr", 9.0, 44.5, 15.0, 50.5, 0], ["rr", 27.0, 44.5, 33.0, 50.5, 0], ["rr", 45.0, 44.5, 51.0, 50.5, 0], ["rr", 54.0, 44.5, 60.0, 50.5, 0], ["rr", 9.0, 34.0, 15.0, 40.0, 0], ["rr", 18.0, 34.0, 24.0, 40.0, 0], ["rr", 36.0, 34.0, 42.0, 40.0, 0], ["rr", 45.0, 34.0, 51.0, 40.0, 0], ["rr", 9.0, 23.5, 15.0, 29.5, 0], ["rr", 27.0, 23.5, 33.0, 29.5, 0], ["rr", 45.0, 23.5, 51.0, 29.5, 0], ["rr", 54.0, 23.5, 60.0, 29.5, 0], ["rr", 18.0, 12.0, 24.0, 18.0, 0], ["rr", 27.0, 12.0, 33.0, 18.0, 0], ["rr", 36.0, 12.0, 42.0, 18.0, 0]], 21.200000000000003, 24.700000000000003]]}, {"name": "Hmatniky", "color": "#444444", "ops": [["+", [["c", 12.0, 58.0, 3.5], ["c", 21.0, 58.0, 3.5], ["c", 30.0, 58.0, 3.5], ["c", 39.0, 58.0, 3.5], ["c", 48.0, 58.0, 3.5], ["c", 57.0, 58.0, 3.5], ["c", 12.0, 47.5, 3.5], ["c", 30.0, 47.5, 3.5], ["c", 48.0, 47.5, 3.5], ["c", 57.0, 47.5, 3.5], ["c", 12.0, 37.0, 3.5], ["c", 21.0, 37.0, 3.5], ["c", 39.0, 37.0, 3.5], ["c", 48.0, 37.0, 3.5], ["c", 12.0, 26.5, 3.5], ["c", 30.0, 26.5, 3.5], ["c", 48.0, 26.5, 3.5], ["c", 57.0, 26.5, 3.5], ["c", 21.0, 15.0, 3.5], ["c", 30.0, 15.0, 3.5], ["c", 39.0, 15.0, 3.5]], 24.700000000000003, 27.200000000000003]]}, {"name": "LED_0603", "color": "#f2f2f2", "ops": [["+", [["rr", 73.7, 60.1, 75.3, 60.9, 0], ["rr", 77.7, 60.1, 79.3, 60.9, 0], ["rr", 81.7, 60.1, 83.3, 60.9, 0], ["rr", 67.7, 51.1, 69.3, 51.9, 0], ["rr", 71.7, 51.1, 73.3, 51.9, 0], ["rr", 75.7, 51.1, 77.3, 51.9, 0], ["rr", 79.7, 51.1, 81.3, 51.9, 0], ["rr", 83.7, 51.1, 85.3, 51.9, 0], ["rr", 87.7, 51.1, 89.3, 51.9, 0], ["rr", 71.7, 42.6, 73.3, 43.4, 0], ["rr", 75.7, 42.6, 77.3, 43.4, 0], ["rr", 79.7, 42.6, 81.3, 43.4, 0], ["rr", 83.7, 42.6, 85.3, 43.4, 0], ["rr", 67.7, 34.1, 69.3, 34.9, 0], ["rr", 71.7, 34.1, 73.3, 34.9, 0], ["rr", 75.7, 34.1, 77.3, 34.9, 0], ["rr", 79.7, 34.1, 81.3, 34.9, 0], ["rr", 83.7, 34.1, 85.3, 34.9, 0], ["rr", 87.7, 34.1, 89.3, 34.9, 0]], 23.500000000000004, 24.100000000000005]]}, {"name": "74HC595_SOIC16", "color": "#202020", "ops": [["+", [["rr", 73.55, 56.55, 83.45, 60.45, 0], ["rr", 73.55, 44.55, 83.45, 48.45, 0], ["rr", 73.55, 32.55, 83.45, 36.45, 0]], 17.85, 19.6]]}, {"name": "Konektory_A_B", "color": "#111111", "ops": [["+", [["rr", 94.33, 23.029999999999998, 96.86999999999999, 38.27, 0], ["rr", 15.309999999999999, 19.98, 40.71, 22.52, 0]], 8.6, 19.6]]}, {"name": "Listy_A_C", "color": "#111111", "ops": [["+", [["rr", 63.93, 32.79, 66.47, 63.27, 0], ["rr", 90.53, 32.33, 93.07, 60.27, 0]], 21.200000000000003, 22.700000000000003]]}]""")

def cm(v): return v / 10.0

def run(context):
    ui = None
    try:
        app = adsk.core.Application.get(); ui = app.userInterface
        doc = app.documents.add(adsk.core.DocumentTypes.FusionDesignDocumentType)
        design = adsk.fusion.Design.cast(app.activeProduct)
        design.designType = adsk.fusion.DesignTypes.ParametricDesignType
        root = design.rootComponent
        fails = []
        for part in PARTS:
            occ = root.occurrences.addNewComponent(adsk.core.Matrix3D.create())
            comp = occ.component; comp.name = part['name']
            bodies = []
            for op, shapes, z0, z1 in part['ops']:
                try:
                    if abs(z0) < 1e-9:
                        pl = comp.xYConstructionPlane
                    else:
                        pin = comp.constructionPlanes.createInput()
                        pin.setByOffset(comp.xYConstructionPlane, adsk.core.ValueInput.createByReal(cm(z0)))
                        pl = comp.constructionPlanes.add(pin); pl.isLightBulbOn = False
                    sk = comp.sketches.add(pl)
                    for s in shapes: draw(sk, s, cm(z0))
                    profs = adsk.core.ObjectCollection.create()
                    for i in range(sk.profiles.count): profs.add(sk.profiles.item(i))
                    if profs.count == 0: fails.append(part['name']); continue
                    ext = comp.features.extrudeFeatures
                    kind = adsk.fusion.FeatureOperations.NewBodyFeatureOperation if op == '+' else adsk.fusion.FeatureOperations.CutFeatureOperation
                    inp = ext.createInput(profs, kind)
                    inp.setDistanceExtent(False, adsk.core.ValueInput.createByReal(cm(z1 - z0)))
                    if op == '-':
                        inp.participantBodies = [b for b in bodies if b.isValid]
                    f = ext.add(inp)
                    if op == '+':
                        for i in range(f.bodies.count): bodies.append(f.bodies.item(i))
                    sk.isVisible = False
                except:
                    fails.append(part['name'] + ': ' + traceback.format_exc().splitlines()[-1])
                if op == '+' and part.get('merge') and len(bodies) > 1:
                    tools = adsk.core.ObjectCollection.create()
                    for b in bodies[1:]: tools.add(b)
                    ci = comp.features.combineFeatures.createInput(bodies[0], tools)
                    ci.operation = adsk.fusion.FeatureOperations.JoinFeatureOperation
                    comp.features.combineFeatures.add(ci)
                    bodies = [bodies[0]]
            for b in comp.bRepBodies: b.name = part['name']
        app.activeViewport.fit()
        msg = 'Hotovo: %d dílů.' % len(PARTS)
        if fails: msg += '\nProblémy:\n' + '\n'.join(fails[:20])
        ui.messageBox(msg)
    except:
        if ui: ui.messageBox('Chyba:\n{}'.format(traceback.format_exc()))

def P3(sk, x, y, z):
    return sk.modelToSketchSpace(adsk.core.Point3D.create(cm(x), cm(y), z))

def draw(sk, s, z):
    lines = sk.sketchCurves.sketchLines
    if s[0] == 'c':
        c = P3(sk, s[1], s[2], z)
        sk.sketchCurves.sketchCircles.addByCenterRadius(c, cm(s[3]) / 2.0)
    elif s[0] == 'rr':
        x0, y0, x1, y1, r = s[1:]
        rect = lines.addTwoPointRectangle(P3(sk, x0, y0, z), P3(sk, x1, y1, z))
        if r > 0:
            ls = [rect.item(i) for i in range(rect.count)]
            corners = [P3(sk, x0, y0, z), P3(sk, x1, y0, z), P3(sk, x1, y1, z), P3(sk, x0, y1, z)]
            for cp in corners:
                hit = [l for l in ls if l.isValid and (l.startSketchPoint.geometry.distanceTo(cp) < 1e-6 or l.endSketchPoint.geometry.distanceTo(cp) < 1e-6)]
                if len(hit) == 2:
                    sk.sketchCurves.sketchArcs.addFillet(hit[0], cp, hit[1], cp, cm(r))
    elif s[0] == 'poly':
        pts = [P3(sk, x, y, z) for x, y in s[1]]
        prev = None; first = None
        for i in range(len(pts)):
            a = prev.endSketchPoint if prev else pts[i]
            b = first.startSketchPoint if (i == len(pts) - 1 and first) else pts[(i + 1) % len(pts)]
            l = lines.addByTwoPoints(a, b)
            if first is None: first = l
            prev = l
