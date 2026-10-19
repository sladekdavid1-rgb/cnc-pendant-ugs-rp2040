# Changelog

## v1.0.1 – 2026-10-19

- Obrácený směr enkodéru (`ENC_INVERT = true`) podle hotového kusu.
- Oprava firmwaru (v11.1 a v10.1): HOME, RTZ, PROBE a NULA X/Y/Z fungují i bez zvolené osy. Zvolená osa je potřeba jen pro pojezd (šipky a enkodér). Dřív tato tlačítka bez zvolené osy nereagovala.

## v1.0 – 2026-10-19

První kompletní verze, postavená a odzkoušená.

- Desky A (tlačítka, 74HC595), B (RP2040, enkodér), C (LED): KiCad 10, gerbery, G-kódy pro FlatCAM/Candle
- Přípravek z MDF s kolíčky 88 × 58 mm pro oboustranné desky
- Hliníkový panel: frézování (Fusion), popisy laserem (LightBurn, SVG s nulou)
- Krabička, šestihranné sloupky a knoflík pro 3D tisk
- Firmware v10 (ověřený) a v11 (pojistka podržením, diagonály, zrychlení enkodéru, stmívání, testovací režim)
- Zkratky pro UGS, návod CZ (HTML/PDF), README EN, kontrola DPS a nástroje pro opakovanou kontrolu
