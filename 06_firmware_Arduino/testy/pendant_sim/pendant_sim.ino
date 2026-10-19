/*
  CNC pendant – SIMULACE NASTAVENÍ (jen LED, nic se neposílá do PC ani do UGS)

  Tlačítka mění hodnoty a LED ukazují stav stejně jako v hlavním programu:
    OSA X / OSA Y / OSA Z  – volba osy (svítí LED osy), druhý stisk osu zruší
    X+ / X−, Y+ / Y−       – krok XY o stupeň nahoru / dolů   (0.05 / 0.1 / 1 / 10 / 50 / 100 mm)
    Z+ / Z−                – krok Z o stupeň nahoru / dolů    (0.05 / 0.1 / 1 / 10 mm)
    F+ / F−                – rychlost nahoru / dolů           (10 / 100 / 1000 / 2000 / 3000 / MAX)
    tlačítko enkodéru      – krátce = krok o stupeň výš, dlouze (>0,6 s) = níž
                             (při zvolené ose Z mění krok Z, jinak krok XY – jako v hlavním programu)
    otočení enkodéru       – jen výpis do Serial Monitoru (krok ± v ose)
    ostatní tlačítka       – LED se nemění, stisk se jen vypíše do Serial Monitoru
  Stav se vypisuje do Serial Monitoru (115200 Bd).

  Arduino IDE: deska "Raspberry Pi Pico" (arduino-pico)
*/

// ---- piny (stejné jako cnc_pendant.ino) ----
const uint8_t ROW_PINS[3] = { 2, 3, 4 };
const uint8_t COL_PINS[7] = { 5, 6, 7, 8, 9, 10, 11 };
const uint8_t PIN_ENC_A = 15, PIN_ENC_B = 14, PIN_ENC_SW = 13;
const uint8_t PIN_595_DATA = 18, PIN_595_CLK = 19, PIN_595_LATCH = 20, PIN_595_OE = 21;
const uint16_t ROW_SETTLE_US = 500, DEBOUNCE_MS = 20, LONG_PRESS_MS = 600;
const uint8_t EDGES_PER_DETENT = 4;

// ---- tlačítka (matice: řádek r, sloupec c = SW(r*7+c+1)) ----
enum Act : uint8_t {
  A_NONE, A_JOG_XM, A_JOG_XP, A_JOG_YP, A_JOG_YM, A_JOG_ZP, A_JOG_ZM,
  A_HOLD, A_START, A_STOP, A_UNLOCK, A_SEL_X, A_SEL_Y, A_SEL_Z,
  A_FEED_DN, A_FEED_UP, A_ZERO_X, A_ZERO_Y, A_ZERO_Z, A_PROBE, A_HOME, A_RTZ
};
const Act KEYMAP[3][7] = {
  { A_UNLOCK, A_START,  A_HOLD,   A_STOP,   A_HOME,   A_RTZ,   A_ZERO_X },
  { A_JOG_YP, A_JOG_ZP, A_FEED_UP,A_ZERO_Y, A_JOG_XM, A_JOG_XP,A_PROBE  },
  { A_ZERO_Z, A_JOG_YM, A_JOG_ZM, A_FEED_DN,A_SEL_X,  A_SEL_Y, A_SEL_Z  },
};
const char* NAME[3][7] = {
  { "UNLOCK", "START", "HOLD", "STOP", "HOME", "RTZ", "NULA X" },
  { "Y+", "Z+", "F+", "NULA Y", "X-", "X+", "PROBE" },
  { "NULA Z", "Y-", "Z-", "F-", "OSA X", "OSA Y", "OSA Z" },
};

// ---- hodnoty ----
const char* XY_TXT[6]   = { "0.05", "0.1", "1", "10", "50", "100" };
const char* Z_TXT[4]    = { "0.05", "0.1", "1", "10" };
const char* FEED_TXT[6] = { "10", "100", "1000", "2000", "3000", "MAX" };
int xyLevel = 1, zLevel = 0, feedLevel = 2, axis = 0;   // axis 0 = žádná, 1 X, 2 Y, 3 Z

// ---- LED (bit = 8*registr + výstup) ----
const uint8_t LEDB_XY = 1, LEDB_FEED = 9, LEDB_Z = 17, LEDB_AXIS = 21;
uint32_t ledAll() {
  uint32_t v = 0;
  for (int i = 0; i < 3; i++) v |= 1UL << (LEDB_AXIS + i);
  for (int i = 0; i < 6; i++) v |= (1UL << (LEDB_XY + i)) | (1UL << (LEDB_FEED + i));
  for (int i = 0; i < 4; i++) v |= 1UL << (LEDB_Z + i);
  return v;
}
void writeLeds(uint32_t v) {
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 16) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 8) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, v & 0xFF);
  digitalWrite(PIN_595_LATCH, HIGH);
}
uint32_t flashUntil = 0;
void showState(uint32_t now) {
  if (now < flashUntil) { writeLeds(ledAll()); return; }
  uint32_t v = (1UL << (LEDB_XY + xyLevel)) | (1UL << (LEDB_Z + zLevel)) | (1UL << (LEDB_FEED + feedLevel));
  if (axis) v |= 1UL << (LEDB_AXIS + axis - 1);
  writeLeds(v);
}
void printState() {
  Serial.print("osa: "); Serial.print(axis == 0 ? "-" : axis == 1 ? "X" : axis == 2 ? "Y" : "Z");
  Serial.print("   krok XY: "); Serial.print(XY_TXT[xyLevel]);
  Serial.print(" mm   krok Z: "); Serial.print(Z_TXT[zLevel]);
  Serial.print(" mm   rychlost: "); Serial.println(FEED_TXT[feedLevel]);
}

// ---- enkodér ----
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
bool swStable = HIGH, swRaw = HIGH;
uint32_t swChanged = 0, swPressedAt = 0;

void onPress(uint8_t r, uint8_t c, uint32_t now) {
  Serial.print("[");
  Serial.print(NAME[r][c]);
  Serial.print("]  ");
  switch (KEYMAP[r][c]) {
    case A_SEL_X:   axis = (axis == 1) ? 0 : 1; break;
    case A_SEL_Y:   axis = (axis == 2) ? 0 : 2; break;
    case A_SEL_Z:   axis = (axis == 3) ? 0 : 3; break;
    case A_JOG_XP: case A_JOG_YP:  if (xyLevel < 5) xyLevel++; break;
    case A_JOG_XM: case A_JOG_YM:  if (xyLevel > 0) xyLevel--; break;
    case A_JOG_ZP:  if (zLevel < 3) zLevel++; break;
    case A_JOG_ZM:  if (zLevel > 0) zLevel--; break;
    case A_FEED_UP: if (feedLevel < 5) feedLevel++; break;
    case A_FEED_DN: if (feedLevel > 0) feedLevel--; break;
    default:        break;                           // ostatní tlačítka: jen výpis do Serial Monitoru
  }
  printState();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_ENC_A, INPUT_PULLUP); pinMode(PIN_ENC_B, INPUT_PULLUP); pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_595_DATA, OUTPUT); pinMode(PIN_595_CLK, OUTPUT); pinMode(PIN_595_LATCH, OUTPUT);
  for (uint8_t r = 0; r < 3; r++) pinMode(ROW_PINS[r], INPUT);
  for (uint8_t c = 0; c < 7; c++) pinMode(COL_PINS[c], INPUT_PULLUP);
  writeLeds(0);
  pinMode(PIN_595_OE, OUTPUT); digitalWrite(PIN_595_OE, LOW);
  encState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encIsr, CHANGE);
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
        if (p) onPress(r, c, now);
      }
    }
    pinMode(ROW_PINS[r], INPUT);
  }
  // tlačítko enkodéru: krátce = krok výš, dlouze = krok níž
  bool s = digitalRead(PIN_ENC_SW);
  if (s != swRaw) { swRaw = s; swChanged = now; }
  if (s != swStable && now - swChanged >= DEBOUNCE_MS) {
    swStable = s;
    if (s == LOW) swPressedAt = now;
    else {
      bool lng = now - swPressedAt >= LONG_PRESS_MS;
      Serial.print(lng ? "[ENKODER dlouze]  " : "[ENKODER kratce]  ");
      if (axis == 3) zLevel  = lng ? max(0, zLevel - 1)  : min(3, zLevel + 1);
      else           xyLevel = lng ? max(0, xyLevel - 1) : min(5, xyLevel + 1);
      printState();
    }
  }
  // otočení enkodéru – jen výpis
  int32_t e; noInterrupts(); e = encEdges; interrupts();
  int32_t d = e / EDGES_PER_DETENT;
  if (d != 0) {
    noInterrupts(); encEdges -= d * EDGES_PER_DETENT; interrupts();
    Serial.print("[ENKODER "); Serial.print(d > 0 ? "+" : ""); Serial.print(d); Serial.print("]  ");
    if (axis == 0) Serial.println("osa nezvolena - stroj by se nepohnul");
    else {
      Serial.print("jog "); Serial.print(axis == 1 ? "X" : axis == 2 ? "Y" : "Z");
      Serial.print(d > 0 ? " +" : " -"); Serial.print(axis == 3 ? Z_TXT[zLevel] : XY_TXT[xyLevel]); Serial.println(" mm");
    }
  }
  showState(now);
}
