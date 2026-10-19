# Stav projektu a postup pro budoucí úpravy

**Verze:** v1.0 (19. 10. 2026)
**Stav:** dokončeno a archivováno. Pendant je postavený a funguje. Repozitář je připravený na další úpravy.

## Co je hotové a ověřené

- Desky A, B, C jsou vyrobené a osazené. Všech 21 tlačítek a 19 LED funguje.
- Zkratky pro UGS jsou nastavené a odzkoušené.
- Panel je vyfrézovaný z hliníku, nastříkaný černým lakem a popsaný laserem.
- Krabička, šestihranné sloupky a knoflík jsou vytištěné a sedí.
- Podklady prošly kontrolou proti KiCadu: schéma ↔ DPS, DRC, gerbery, simulace izolace, vrtání a ořezy (viz `KONTROLA_DPS.md`).

## Otevřené body

| Bod | Poznámka |
|---|---|
| Firmware v11 na hardwaru | Prošel simulací na PC. Ověřit na stroji: pojistku podržením, diagonály, zrychlení enkodéru a stmívání LED. Záložní verze je v10. |
| Odpory LED R301–R304 | Výchozí hodnoty. Doladit podle použitých LED a požadovaného jasu. |

## Nápady na další verzi (zatím nerealizované)

- Desku A routovat bez průchodů pod pouzdry 595 a pod lištami. Tam se vadná prokovka nedá opravit a musel se nahradit drátkem.
- Na desku B přidat externí pull-upy enkodéru. Teď stačí vnitřní pull-upy RP2040.
- Přímý (offline) režim: pendant posílá G-kód rovnou do GRBL bez PC. Hodilo by se to i pro korekce posuvu a otáček a pro LED se stavem stroje.
- Makra UGS na druhé vrstvě tlačítek (vřeteno, bezpečná výška, výměna nástroje), pokud UGS dovolí makrům přiřadit klávesové zkratky.

## Jak udělat úpravu

### Elektronika (DPS)

1. Uprav schéma a DPS v KiCadu 10 (`01_DPS_KiCad`). Na konci spusť *Update PCB from Schematic*, *Fill All Zones* a DRC.
2. Vyexportuj gerbery a vrtání do `02_DPS_frezovani/<deska>/gerber` (stejné názvy souborů jako teď).
3. Ve FlatCAMu (projekty `*.FlatPrj`) vygeneruj G-kódy a ulož je pod stávajícími názvy (A1…A6, B1…B4, C1…C3). Zachovej nuly a zrcadlení podle `KONTROLA_DPS.md`.
4. Spusť kontrolu z kořene repozitáře:
   ```
   pip install numpy scipy pillow matplotlib
   python nastroje/kontrola.py
   ```
   Na konci musí napsat **„vše v pořádku“**. Kontroluje propojení, zkraty, mezery, shodu schématu s DPS, gerbery, simulaci izolace a vrtání.
5. Překresli náhledy desek: `python nastroje/render.py` (zapisuje do `00_obrazky`).
6. Dopiš výsledky do `KONTROLA_DPS.md` a změnu do `CHANGELOG.md`.

### Firmware

- Úpravy dělej v `06_firmware_Arduino/cnc_pendant_v11` (Arduino IDE 2, jádro arduino-pico, USB Stack „Pico SDK“).
- Při změně klávesových zkratek uprav i soubory v `07_UGS_zkratky/NetBeans`. Název souboru je zkratka (`S-` = Shift, `D-` = Ctrl, `SD-` = Ctrl+Shift), obsah je ID akce v UGS.

### Panel a mechanika

- Popisy panelu: `04_panel_cela/2_laser_paleni/pendant_laser_vse_v_jednom.svg`. Barvy jsou vrstvy LightBurnu a tečka vlevo dole je nula.
- Krabička: `05_krabicka_3D_tisk/CNCpendant.step`. Parametrický model je v `pendant_krabicka.scad`, skript pro Fusion v `pendant_krabicka_fusion`.

### Návod

- Uprav `NAVOD.html`. PDF vytvoříš tak, že HTML otevřeš v prohlížeči a dáš Tisk → Uložit jako PDF (A4, s pozadím).
- Hotové změny commitni, založ nový tag (např. `v1.1`) a v GitHubu k němu release s novým ZIPem.

## Nástroje (`nastroje/`)

| Soubor | Účel |
|---|---|
| `kontrola.py` | spustí všechny kontroly najednou |
| `pcbcheck.py` | načtení DPS z KiCadu, rastr mědi, DRC, propojení, porovnání se schématem |
| `sch_netlist.py` | netlist ze schématu KiCad |
| `isosim.py` | simulace izolačního frézování z G-kódu |
| `render.py` | náhledy desek do `00_obrazky` |
| `sx.py` | parser S-výrazů KiCadu |
