# Kontrola DPS – 19. 10. 2026

Zdroj: soubory KiCad v `01_DPS_KiCad` (A: `pendant_A_horni`, B: `pendant_B_spodni`, C: `pendant_C_led`).
Kontrola proběhla vlastními skripty (rastr 0,025 mm), nezávisle na KiCadu.
Citlivost kontroly jsem ověřil uměle zanesenými chybami: odebraná cesta se ukázala jako přerušení, posunutá cesta jako zkrat.

## 1. Schéma ↔ DPS

| Deska | Součástky | Sítě | Rozdíly schéma/DPS | Nepřipojené piny ve schématu |
|---|---|---|---|---|
| A | 58 | 64 | 0 | 0 |
| B | 27 | 19 | 0 | 0 |
| C | 25 | 24 | 0 | 0 |

U každé součástky sedí reference, hodnota i pouzdro a v každé síti jsou stejné piny.

## 2. Měď v DPS (DRC)

| Deska | Cesty / via | Zkraty | Mezery < 0,2 mm | Nepropojené sítě |
|---|---|---|---|---|
| A (2 vrstvy + zemní plocha) | 616 / 37 | 0 | 0 | 0 |
| B (1 vrstva B.Cu + zemní plocha) | 146 / 0 | 0 | 0 | 0 |
| C (1 vrstva F.Cu) | 111 / 0 | 0 | 0 | 0 |

Deska A nemá prokovené otvory. 37 průchodů je potřeba protáhnout drátkem a 43 vývodů pájet i shora:
vývod 2 všech tlačítek (u SW1, SW9, SW10, SW16, SW17 oba vývody 2), J1.6, J2.2 a J2.4–10, J3.1, 2, 5, 6 a J4.1, 2, 6, 7.

## 3. Gerbery ↔ DPS

| Gerber | Cesty | Pady/via |
|---|---|---|
| A F_Cu | 186 = 186 | 157 = 157 |
| A B_Cu | 430 = 430 | 257 = 257 |
| B B_Cu | 146 = 146 | 101 = 101 |
| C F_Cu | 108 = 108 | 66 = 66 (rozdíl jen zaokrouhlení 0,005 mm) |

Gerbery v `02_DPS_frezovani/*/gerber` odpovídají aktuálním DPS.

## 4. G-kódy ↔ DPS

### Izolace (simulace frézy podle průměru v hlavičce souboru)

| Program | Fréza | Mapování na DPS | Zkraty | Přerušené sítě |
|---|---|---|---|---|
| A1 spodní měď + A2 horní měď (dohromady, přes průchody a vývody) | 0,16 / 0,18 | A1 zrcadleno přes vodorovnou osu | 0 | 0 |
| B2 měď | 0,18 | zrcadleno přes vodorovnou osu | 0 | 0 |
| C1 měď | 0,18 | přímo | 0 | 0 |

Fréza zasahuje do mědi jen na okraji zemní plochy (FlatCAM obkresluje gerber, KiCad má plochu spočítanou vlastní cestou). U signálových cest jde o setiny mm² a nevadí to.

### Vrtání

| Program | Děr v DPS | V programu | Chybí / navíc |
|---|---|---|---|
| A3 vrtání 0,4 | 37 | 37 | 0 / 0 |
| A4 vrtání 1,0 (díry 1,0 i 1,1) | 120 | 120 | 0 / 0 |
| A0 rohové díry (2,5 → ručně 3,2) | 4 | 4 | 0 / 0 |
| B3 vrtání 1,0 | 61 | 61 | 0 / 0 |
| B1 rohové díry | 4 | 4 | 0 / 0 |
| C2 vrtání 1,0 | 20 | 20 | 0 / 0 |

### Obrysy a kapsa

- A6, B4 a C3: střed frézy 1,8 mm jede 1,05–1,08 mm od obrysu, celý mimo desku. Hotová deska tak vyjde asi o 0,15 mm na každé straně větší než v KiCadu.
- A5 kapsa Ø10: střed X72 Y12 odpovídá kruhu Ø10 v Edge.Cuts desky A (KiCad 116; 90).

### Nuly

| Deska | X0 Y0 |
|---|---|
| A | střed levé dolní montážní díry (= kolíček přípravku) |
| B | levý dolní roh desky (3,5 mm vlevo a pod dírou kolíčku) |
| C | levý dolní roh obrysu |

## 5. Propojení desek (z DPS)

| Konektor | Piny |
|---|---|
| A J1 = B J101 | OE · RCLK · SRCLK · SER · GND · +3V3 |
| A J2 = B J103 | ROW0 · ROW1 · ROW2 · COL0 … COL6 |
| A J3 = C J301 | LD1 · LD6 · LD5 · LD4 · LD11 · LD10 · GND · LD16 · LD15 · LD14 |
| A J4 = C J302 | LD3 · LD2 · LD7 · LD8 · LD9 · LD12 · LD13 · LD17 · LD18 · LD19 |
