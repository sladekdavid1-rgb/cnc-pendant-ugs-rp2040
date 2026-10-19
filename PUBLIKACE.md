# Podklady pro zveřejnění

Hotové texty ke zkopírování. Fotky skutečného pendantu přidej všude – udělají nejvíc.

---

## 1. GitHub

**Název repozitáře:** `cnc-pendant-ugs-rp2040`

**Popis (About):**
USB jog pendant for GRBL/UGS: RP2040 keyboard, 21 buttons, encoder, 19 LEDs. Home-made PCBs (KiCad + FlatCAM), laser-engraved panel, 3D-printed case.

**Topics:** `cnc` `grbl` `ugs` `universal-gcode-sender` `pendant` `rp2040` `raspberry-pi-pico` `kicad` `flatcam` `pcb-milling` `3018` `lightburn` `arduino`

**Postup:**
1. github.com → New repository → název výše → Public → bez README (máš vlastní).
2. Nahraj obsah složky `CNC_pendant_kompletni` (přes „uploading an existing file“ – přetáhni všechny složky; GitHub Desktop je pohodlnější).
3. Releases → Create a new release → tag `v1.0` → přilož celý ZIP, ať si ho lidi stáhnou jedním klikem.

---

## 2. Hackaday.io (anglicky)

**Project name:** CNC Pendant for UGS – RP2040 USB keyboard jog controller

**Summary (krátký popis):**
A home-made jog pendant for 3018-class GRBL machines. It is a USB keyboard that drives Universal Gcode Sender through F13–F24 shortcuts – no drivers, no GRBL changes.

**Description:**
I wanted a physical jog controller for my Lunyee 3018 Pro Ultra running GRBL and Universal Gcode Sender, and I wanted to make every part of it on the machine itself.

The pendant is a Laskakit Ultimate Pi Pico (RP2040) acting as a USB HID keyboard. Each button sends an F13–F24 shortcut (with Shift/Ctrl/Ctrl+Shift) that is bound to a UGS action, so it works with stock UGS and stock GRBL.

- 21 buttons in a 3×7 diode matrix: axis select, jog X/Y/Z, feed, start/hold/stop/unlock, home, return to zero, probe, zero X/Y/Z
- EC11 encoder with acceleration – spin fast and the step grows temporarily
- 19 LEDs through three 74HC595 show axis, XY step, Z step and jog feed; values are set in UGS absolutely, so LEDs never drift
- Safety: critical buttons need a 0.5 s hold, jogging is locked until an axis is selected, STOP jumps the queue
- Diagonal jog by pressing X and Y together, LED dimming, built-in hardware test mode

Manufacturing: three PCBs isolation-milled with FlatCAM + Candle height maps on an MDF registration jig (pins on an 88×58 mm grid let the double-sided board flip accurately), aluminium front panel milled and then engraved through black paint with a diode laser in LightBurn, enclosure 3D printed.

Everything is open: KiCad projects, Gerbers, G-code in machining order, jig, laser files, STEP, firmware and the UGS keymap. Hardware/docs CC BY-SA 4.0, firmware MIT.

*Designed and built with the help of an AI assistant (Claude) for the schematics, firmware and documentation; everything was made and tested on real hardware.*

**Components:** Laskakit Ultimate Pi Pico RP2040 · EC11E encoder · 74HC595 ×3 · 1N4148W ×21 · 6×6 mm tactile switch ×21 · LED 0603 ×19

**Instructions:** odkaz na GitHub (README) a NAVOD.pdf (v češtině).

---

## 3. Printables (anglicky)

**Model name:** CNC Pendant enclosure + knob + standoffs (UGS / GRBL jog controller)

**Description:**
Printable parts for an open-source USB jog pendant for 3018-class CNC machines (electronics, firmware and full build guide on GitHub – link).

- Enclosure (STEP – opens directly in Bambu Studio, Orca and PrusaSlicer), print bottom down
- Hex standoffs 5.5 mm AF × 11 mm – the M3 screw cuts its own thread (hole 2.6 mm), hold them with a 5.5 mm nut driver
- Round spacers Ø6 × 3.5 mm
- Low-profile Ø25 knob for EC11 encoders, versions for 9–14 mm of shaft above the panel, D-shaft or knurled shaft

**Print settings:** PETG or PLA, 0.2 mm layers; standoffs upright with 100 % infill; knob bottom down, no supports.

**Licence:** CC BY-SA 4.0
*Designed and built with the help of an AI assistant (Claude) for the schematics, firmware and documentation; everything was made and tested on real hardware.*

**Soubory k nahrání:** `05_krabicka_3D_tisk/CNCpendant.step`, `distancni_sloupky/distance_sada_5+5ks.stl`, `knoflik/*.stl` (stačí D_osa a vroubkovana_osa 12–14 mm), náhled `knoflik_nahled.png`.

---

## 4. forum.cnc.cz (česky)

**Název vlákna:** Pendant (ruční ovladač) pro UGS/GRBL na RP2040 – vše doma na 3018, podklady volně

**Text:**
Ahoj, postavil jsem si ruční ovladač k frézce 3018 Pro Ultra s GRBL a UGS a dávám podklady volně k dispozici.

Funguje jako USB klávesnice (RP2040 – Laskakit Ultimate Pi Pico), takže v UGS stačí nahrát klávesové zkratky (soubory jsou přiložené) a nic dalšího se neinstaluje. 21 tlačítek (osy, jog, F±, start/hold/stop/unlock, home, RTZ, probe, nuly), enkodér se zrychlením, 19 LED ukazuje osu, krok XY, krok Z a rychlost. Důležitá tlačítka chce podržet 0,5 s, bez zvolené osy je jog zamčený.

Všechno je vyrobené na té samé 3018: tři DPS frézované přes FlatCAM a Candle (výšková mapa) na přípravku z MDF s kolíčky, hliníkový panel vyfrézovaný a popsaný laserem přes černý lak (LightBurn), krabička z tiskárny.

V balíku jsou projekty KiCad, gerbery, G-kódy seřazené podle postupu, přípravek, soubory pro laser, STEP krabičky, firmware a podrobný návod v češtině.
Odkaz: (GitHub)

Licence: hardware a dokumentace CC BY-SA 4.0, firmware MIT. Budu rád za připomínky a fotky, pokud si to někdo postaví.

Při návrhu schémat, firmwaru a dokumentace mi pomáhala AI (Claude); všechno je vyrobené a odzkoušené na skutečném kusu.

---

## Kontrola před zveřejněním

- [ ] Jméno autora v `LICENSE` souborech a v README (© 2026 David Sládek) – oprav, pokud chceš jinak nebo přezdívku.
- [ ] Projekty FlatCAM (`*.FlatPrj`) obsahují cesty k původním gerberům ve tvém PC (`C:/Users/dawe1/…`) – nic citlivého, jen uživatelské jméno Windows. Kdo projekt otevře, gerbery si načte z `gerber/`.
- [ ] Přidej fotky hotového pendantu (např. do `00_obrazky/foto_*.jpg`) a doplň je do README.
- [ ] Až ověříš firmware v11 na hardwaru, uprav v README sekci *Status*.
