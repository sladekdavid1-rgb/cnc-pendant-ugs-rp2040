/*
  CNC pendant – TEST LED (19 LED přes 3× 74HC595)
  Nic nepíše na klávesnici – jen bliká LED (a vypisuje do Serial Monitoru 115200).

  VSE_NARAZ = true (výchozí): všech 19 LED trvale svítí.
  VSE_NARAZ = false: program dokola opakuje animaci:
   1) všechny LED svítí 2 s
   2) "běžící světlo" ve všech 4 řadách naráz (každá řada má svůj bod)
   3) každá řada zvlášť: postupně se rozsvítí odleva doprava, pak zhasne
   4) všechny LED jedna po druhé v pořadí LED1 … LED19
   5) blikání všech 3×

  Řady na panelu (LED číslo -> výstup 595):
   OSA      LED1–3   : U6 QF..QH
   KROK XY  LED4–9   : U4 QB..QG
   KROK Z   LED10–13 : U6 QB..QE
   RYCHLOST LED14–19 : U5 QB..QG

  Arduino IDE: deska "Raspberry Pi Pico" (arduino-pico)
*/

const uint8_t PIN_595_DATA = 18;   // SER
const uint8_t PIN_595_CLK  = 19;   // SRCLK
const uint8_t PIN_595_LATCH = 20;  // RCLK
const uint8_t PIN_595_OE   = 21;   // OE (LOW = výstupy zapnuté)

const bool VSE_NARAZ = true;     // true = všechny LED trvale svítí; false = animace v řadách
const uint16_t KROK_MS = 150;      // rychlost animace

// LED1..19 -> bit v řetězci 595 (8*registr + výstup QA=0..QH=7)
const uint8_t LEDBIT[19] = { 21, 22, 23,  1, 2, 3, 4, 5, 6,  17, 18, 19, 20,  9, 10, 11, 12, 13, 14 };

// řady: první LED (index od 0) a počet
struct Rada { uint8_t prvni, pocet; const char* nazev; };
const Rada RADY[4] = { {0, 3, "OSA"}, {3, 6, "KROK XY"}, {9, 4, "KROK Z"}, {13, 6, "RYCHLOST"} };

void zapis(uint32_t v) {
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 16) & 0xFF);   // třetí 595 (U6)
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 8) & 0xFF);    // druhý 595 (U5)
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, v & 0xFF);           // první 595 (U4)
  digitalWrite(PIN_595_LATCH, HIGH);
}
uint32_t led(int i) { return 1UL << LEDBIT[i]; }
uint32_t vsechny() { uint32_t v = 0; for (int i = 0; i < 19; i++) v |= led(i); return v; }

void setup() {
  Serial.begin(115200);
  pinMode(PIN_595_DATA, OUTPUT);
  pinMode(PIN_595_CLK, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);
  zapis(0);
  pinMode(PIN_595_OE, OUTPUT);
  digitalWrite(PIN_595_OE, LOW);
}

void loop() {
  if (VSE_NARAZ) { zapis(vsechny()); delay(1000); return; }

  // 1) všechny
  Serial.println("Vsechny LED");
  zapis(vsechny()); delay(2000);

  // 2) běžící bod ve všech řadách naráz (3 kola)
  Serial.println("Bezici svetlo v radach");
  for (int k = 0; k < 3 * 6; k++) {
    uint32_t v = 0;
    for (int r = 0; r < 4; r++) v |= led(RADY[r].prvni + k % RADY[r].pocet);
    zapis(v); delay(KROK_MS);
  }

  // 3) každá řada zvlášť: plnění a vyprázdnění
  for (int r = 0; r < 4; r++) {
    Serial.print("Rada "); Serial.println(RADY[r].nazev);
    uint32_t v = 0;
    for (int j = 0; j < RADY[r].pocet; j++) { v |= led(RADY[r].prvni + j); zapis(v); delay(KROK_MS); }
    delay(400);
    for (int j = 0; j < RADY[r].pocet; j++) { v &= ~led(RADY[r].prvni + j); zapis(v); delay(KROK_MS); }
  }

  // 4) LED1 … LED19 jedna po druhé
  for (int i = 0; i < 19; i++) {
    Serial.print("LED "); Serial.println(i + 1);
    zapis(led(i)); delay(KROK_MS * 2);
  }

  // 5) blikání
  for (int k = 0; k < 3; k++) { zapis(vsechny()); delay(300); zapis(0); delay(300); }
}
