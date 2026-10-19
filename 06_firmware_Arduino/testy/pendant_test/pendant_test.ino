/*
  CNC pendant – TEST HARDWARU (dočasný program)
  Píše do otevřeného textového editoru (Poznámkový blok) a zároveň do Serial Monitoru:
    - stisk / puštění každého tlačítka matice (název podle panelu, SW číslo, řádek a sloupec)
    - otočení enkodéru (+/-) a stisk jeho tlačítka
    - LED: po startu proběhne „had“ přes všech 19 LED, pak enkodér posouvá jednu svítící LED
           (do editoru se vypíše číslo LED, takže hned vidíš, jestli svítí ta správná)

  Postup: nahraj, otevři Poznámkový blok, klikni do něj a zapni NumLock.
  Čísla se píšou přes numerickou klávesnici a Y/Z se prohazuje, aby text seděl i na české klávesnici.
  Pokud máš anglickou (US) klávesnici ve Windows, nastav QWERTZ = false.

  Arduino IDE: deska "Raspberry Pi Pico" (arduino-pico), USB Stack: "Pico SDK"
*/
#include <Keyboard.h>

const bool QWERTZ = true;          // česká/německá klávesnice ve Windows: prohodit Y a Z
const bool TYPE_RELEASE = false;   // true = psát i puštění tlačítka

// ---- piny (stejné jako cnc_pendant.ino v9, deska B rev 13+) ----
const uint8_t ROW_PINS[3] = { 2, 3, 4 };
const uint8_t COL_PINS[7] = { 5, 6, 7, 8, 9, 10, 11 };
const uint8_t PIN_ENC_A = 15, PIN_ENC_B = 14, PIN_ENC_SW = 13;
const uint8_t PIN_595_DATA = 18, PIN_595_CLK = 19, PIN_595_LATCH = 20, PIN_595_OE = 21;
const uint16_t ROW_SETTLE_US = 500, DEBOUNCE_MS = 20;
const uint8_t EDGES_PER_DETENT = 4;

// názvy tlačítek SW1..SW21 (matice: řádek = (n-1)/7, sloupec = (n-1)%7)
const char* BTN[21] = {
  "UNLOCK", "START", "HOLD", "STOP", "HOME", "RTZ", "NULA X",
  "Y PLUS", "Z PLUS", "F PLUS", "NULA Y", "X MINUS", "X PLUS", "PROBE",
  "NULA Z", "Y MINUS", "Z MINUS", "F MINUS", "OSA X", "OSA Y", "OSA Z"
};
// LED1..19 -> bit v řetězci 595 (8*registr + výstup QA=0..QH=7)
const uint8_t LEDBIT[19] = { 21, 22, 23,  1, 2, 3, 4, 5, 6,  17, 18, 19, 20,  9, 10, 11, 12, 13, 14 };
const char* LEDGRP[19] = { "OSA X", "OSA Y", "OSA Z",
  "XY 1", "XY 2", "XY 3", "XY 4", "XY 5", "XY 6",
  "Z 1", "Z 2", "Z 3", "Z 4",
  "F 1", "F 2", "F 3", "F 4", "F 5", "F 6" };

// ---------------- psaní textu nezávislé na rozložení ----------------
void kbTap(uint8_t k) { Keyboard.press(k); delay(20); Keyboard.release(k); delay(20); }
void kbChar(char c) {
  if (c >= '0' && c <= '9') { kbTap(c == '0' ? KEY_KP_0 : KEY_KP_1 + (c - '1')); return; }
  if (c == '+') { kbTap(KEY_KP_PLUS); return; }
  if (c == '-') { kbTap(KEY_KP_MINUS); return; }
  if (c == '\n') { kbTap(KEY_RETURN); return; }
  if (c == ' ') { kbTap(' '); return; }
  char l = c;
  if (QWERTZ) {
    if (l == 'y') l = 'z'; else if (l == 'z') l = 'y';
    else if (l == 'Y') l = 'Z'; else if (l == 'Z') l = 'Y';
  }
  if ((l >= 'a' && l <= 'z') || (l >= 'A' && l <= 'Z')) { Keyboard.write(l); delay(20); }
  // ostatní znaky se přeskočí (na CZ klávesnici by vyšly jinak)
}
void out(const String& s) {
  Serial.print(s);
  for (unsigned i = 0; i < s.length(); i++) kbChar(s[i]);
}
String two(int n) { return (n < 10 ? "0" : "") + String(n); }

// ---------------- LED ----------------
void writeLeds(uint32_t v) {
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 16) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 8) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, v & 0xFF);
  digitalWrite(PIN_595_LATCH, HIGH);
}
void showLed(int idx) { writeLeds(idx < 0 ? 0 : (1UL << LEDBIT[idx])); }

// ---------------- enkodér ----------------
volatile int32_t encEdges = 0;
volatile uint8_t encState = 0;
const int8_t QDEC[16] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };
void encIsr() {
  uint8_t s = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  encEdges += QDEC[(encState << 2) | s];
  encState = s;
}

bool keyStable[3][7], keyRaw[3][7];
uint32_t keyChanged[3][7];
bool swStable = HIGH, swRaw = HIGH; uint32_t swChanged = 0;
int ledPos = 0; long encTotal = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_ENC_A, INPUT_PULLUP); pinMode(PIN_ENC_B, INPUT_PULLUP); pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_595_DATA, OUTPUT); pinMode(PIN_595_CLK, OUTPUT); pinMode(PIN_595_LATCH, OUTPUT);
  for (uint8_t r = 0; r < 3; r++) pinMode(ROW_PINS[r], INPUT);
  for (uint8_t c = 0; c < 7; c++) pinMode(COL_PINS[c], INPUT_PULLUP);   // vnitřní pull-up: funguje i bez DPS B (holé Pico)
  writeLeds(0);
  pinMode(PIN_595_OE, OUTPUT); digitalWrite(PIN_595_OE, LOW);    // LOW = výstupy 595 zapnuté
  encState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encIsr, CHANGE);
  Keyboard.begin();

  // had přes všechny LED (bez psaní) – 2 kola, pak všechny naráz na 1 s
  for (int k = 0; k < 2; k++) for (int i = 0; i < 19; i++) { showLed(i); delay(150); }
  uint32_t all = 0; for (int i = 0; i < 19; i++) all |= 1UL << LEDBIT[i];
  writeLeds(all); delay(1000); showLed(ledPos);

  delay(1500);  // čas kliknout do editoru
  out("TEST PENDANTU PRIPRAVEN\n");
  out("LED " + two(ledPos + 1) + " " + LEDGRP[ledPos] + "\n");
}

void loop() {
  uint32_t now = millis();
  // matice
  for (uint8_t r = 0; r < 3; r++) {
    pinMode(ROW_PINS[r], OUTPUT); digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(ROW_SETTLE_US);
    for (uint8_t c = 0; c < 7; c++) {
      bool p = digitalRead(COL_PINS[c]) == LOW;
      if (p != keyRaw[r][c]) { keyRaw[r][c] = p; keyChanged[r][c] = now; }
      if (p != keyStable[r][c] && now - keyChanged[r][c] >= DEBOUNCE_MS) {
        keyStable[r][c] = p;
        int n = r * 7 + c + 1;
        if (p) out("SW" + two(n) + " " + BTN[n - 1] + "   R" + String(r) + " C" + String(c) + "\n");
        else if (TYPE_RELEASE) out("SW" + two(n) + " pusteno\n");
      }
    }
    pinMode(ROW_PINS[r], INPUT);
  }
  // tlačítko enkodéru
  bool s = digitalRead(PIN_ENC_SW);
  if (s != swRaw) { swRaw = s; swChanged = now; }
  if (s != swStable && now - swChanged >= DEBOUNCE_MS) {
    swStable = s;
    if (s == LOW) out("ENKODER TLACITKO\n");
  }
  // otočení enkodéru
  int32_t e; noInterrupts(); e = encEdges; interrupts();
  int32_t d = e / EDGES_PER_DETENT;
  if (d != 0) {
    noInterrupts(); encEdges -= d * EDGES_PER_DETENT; interrupts();
    encTotal += d;
    ledPos = ((ledPos + d) % 19 + 19) % 19;
    showLed(ledPos);
    out(String("ENKODER ") + (d > 0 ? "+" : "-") + String(abs(d)) + "  celkem " + String(encTotal) +
        "   LED " + two(ledPos + 1) + " " + LEDGRP[ledPos] + "\n");
  }
}
