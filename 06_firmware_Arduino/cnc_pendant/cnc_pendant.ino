/*
  CNC pendant pro UGS (Universal Gcode Sender) + GRBL 1.1      verze 10.1
  RP2040 (Laskakit Ultimate Pi Pico / Pico) jako USB klávesnice (zkratky F13–F24 v UGS)

  - 21 tlačítek v matici 3 × 7 (dioda 1N4148W u každého,
    sloupce: pull-up 10k + filtr 10n proti GND)             -> 10 pinů
  - 19 LED přes 3× 74HC595 (osy, krok XY, krok Z, rychlost)  -> 3 piny
  - rotační enkodér KY-040 / EC11 s tlačítkem             -> 3 piny

  Krok a rychlost se v UGS nastavují ABSOLUTNĚ, takže LED bar vždy
  odpovídá UGS:
   - krok XY/Z: přímé akce UGS "XY 0.1 … XY 10" a "Z 0.1 … Z 10",
     0.05 = "XY 0.01" a pak 4× "+", 50 = 10 a pak 40× "+", 100 = 10 a "Multiply"
   - rychlost: 6× "Divide" (UGS spadne na 1 mm/min), pak "Multiply"
     na 10/100/1000 a "+1000" na 2000/3000; MAX = 10000 (GRBL omezí na $110)
  Po zapnutí pendant sám nastaví v UGS krok XY 10 mm, krok Z 1 mm a rychlost 1000
  (XY_START / Z_START / FEED_START). LED kroku/rychlosti blikne jen tehdy, když se
  nějaké ťuknutí nevešlo do fronty – pak hodnotu nastav tlačítkem znovu.
  V UGS musí být zapnutý samostatný krok pro Z ("Use separate step size").

  Arduino IDE: deska "Raspberry Pi Pico" (jádro arduino-pico, RP2040),
               Tools -> USB Stack: "Pico SDK"
*/

#include <Keyboard.h>

// ---------------- Piny ----------------
// piny zvolené pro jednovrstvou DPS B (v13)
const uint8_t PIN_ENC_A  = 15;   // CLK  (pin 18 klonu)
const uint8_t PIN_ENC_B  = 14;   // DT   (pin 17)
const uint8_t PIN_ENC_SW = 13;   // SW   (pin 16) (krátce = větší krok, dlouze = menší)

const uint8_t PIN_595_DATA  = 18; // SER   (pin 14 prvního 595)
const uint8_t PIN_595_CLK   = 19; // SRCLK (pin 11 všech)
const uint8_t PIN_595_LATCH = 20; // RCLK  (pin 12 všech)
const uint8_t PIN_595_OE    = 21; // OE    (pin 13 všech, R50 10k na 3V3; LOW = svítí)
const uint8_t LED_BRIGHT    = 255; // jas LED 0..255 (PWM na OE)

const uint8_t ROW_PINS[3] = { 2, 3, 4 };                    // řádky ROW0-2 (aktivní LOW)
const uint8_t COL_PINS[7] = { 5, 6, 7, 8, 9, 10, 11 };      // sloupce COL0-6 (ext. pull-up 10k + 10n)
const uint16_t ROW_SETTLE_US = 500;  // RC sloupce 10k×10n = 0,1 ms → počkat 5τ

// ---------------- Klávesy v UGS ----------------
const uint8_t NOMOD = 0, SHIFT = KEY_LEFT_SHIFT, CTRL = KEY_LEFT_CTRL;
// bez modifikátoru
const uint8_t K_X_PLUS  = KEY_F13, K_X_MINUS = KEY_F14;
const uint8_t K_Y_PLUS  = KEY_F15, K_Y_MINUS = KEY_F16;
const uint8_t K_Z_PLUS  = KEY_F17, K_Z_MINUS = KEY_F18;
const uint8_t K_HOLD      = KEY_F21;   // Pause / Resume
const uint8_t K_START     = KEY_F22;
const uint8_t K_STOP      = KEY_F23;
const uint8_t K_UNLOCK    = KEY_F24;
// Shift +
const uint8_t KS_ZERO_X   = KEY_F13, KS_ZERO_Y = KEY_F14, KS_ZERO_Z = KEY_F15;
const uint8_t KS_PROBE    = KEY_F16, KS_HOME   = KEY_F17, KS_RTZ    = KEY_F18;
const uint8_t KS_FEED_MUL = KEY_F19;   // jog feed ×10
const uint8_t KS_FEED_DIV = KEY_F20;   // jog feed ÷10
const uint8_t KS_FEED_P1K = KEY_F21;   // jog feed +1000
// Ctrl +
const uint8_t KC_XY_STEP[4] = { KEY_F13, KEY_F14, KEY_F15, KEY_F16 }; // XY 0.01 / 0.1 / 1 / 10
const uint8_t KC_XY_MUL   = KEY_F17;   // XY step ×10
const uint8_t KC_XY_PLUS  = KEY_F18;   // XY step + (nad 1 mm +1, pod 0.1 mm +0.01)
const uint8_t KC_Z_STEP[4]  = { KEY_F19, KEY_F20, KEY_F21, KEY_F22 }; // Z 0.01 / 0.1 / 1 / 10
const uint8_t KC_Z_PLUS   = KEY_F23;   // Z step + (pod 0.1 mm po 0.01)

// ---------------- Nastavení ----------------
const uint8_t  EDGES_PER_DETENT = 4;     // EC11/KY-040 obvykle 4, některé 2
const bool     ENC_INVERT       = true;    // směr otáčení enkodéru (true = obráceně – nastaveno podle hotového kusu; když jede opačně, dej false)
const bool     ENC_AXIS_FLIP[4] = { false, true, false, false };  // [-, X, Y, Z] otočit směr jen v dané ose (X podle hotového kusu)
const uint16_t TAP_HOLD_MS      = 30;    // délka ťuknutí (UGS: < 500 ms = krok)
const uint16_t TAP_GAP_MS       = 30;    // mezera mezi ťuknutími ve frontě
const uint16_t ENC_GAP_MS       = 60;    // min. rozestup kroků z enkodéru
const int8_t   MAX_PENDING      = 3;     // nahromaděné kroky navíc se zahodí
const uint16_t DEBOUNCE_MS      = 20;   // HW RC filtr + SW debounce
const uint16_t LONG_PRESS_MS    = 600;
const uint8_t  FEED_RESET_TAPS  = 6;     // 6× ÷10 = jistě 1 mm/min

// Krok XY:  0..5 = 0.05 / 0.1 / 1 / 10 / 50 / 100 mm
// Krok Z:   0..3 = 0.05 / 0.1 / 1 / 10 mm
// Rychlost: 0..5 = 10 / 100 / 1000 / 2000 / 3000 / MAX mm/min
const uint8_t XY_LEVELS = 6, Z_LEVELS = 4, FEED_LEVELS = 6;
// výchozí hodnoty po zapnutí – pendant je sám pošle do UGS (UGS musí být aktivní okno)
const uint8_t  XY_START = 3;              // krok XY 10 mm
const uint8_t  Z_START  = 2;              // krok Z 1 mm
const uint8_t  FEED_START = 2;            // rychlost 1000 mm/min
const uint16_t INIT_DELAY_MS = 2500;      // počkat, až Windows klávesnici rozpozná
uint8_t xyLevel = XY_START, zLevel = Z_START, feedLevel = FEED_START;
bool    xySynced = true, zSynced = true, feedSynced = true;   // LED hned svítí trvale
bool    initSent = false;

// ---------------- Matice tlačítek ----------------
enum Act : uint8_t {
  A_NONE, A_JOG_XM, A_JOG_XP, A_JOG_YP, A_JOG_YM, A_JOG_ZP, A_JOG_ZM,
  A_HOLD, A_START, A_STOP, A_UNLOCK, A_SEL_X, A_SEL_Y, A_SEL_Z,
  A_FEED_DN, A_FEED_UP, A_ZERO_X, A_ZERO_Y, A_ZERO_Z, A_PROBE, A_HOME, A_RTZ
};

// matice: řádek r, sloupec c = tlačítko SW(r*7+c+1) ve schématu
const Act KEYMAP[3][7] = {
  { A_UNLOCK, A_START,  A_HOLD,   A_STOP,  A_HOME,   A_RTZ,    A_ZERO_X  },   // ROW0
  { A_JOG_YP, A_JOG_ZP, A_FEED_UP,A_ZERO_Y,A_JOG_XM, A_JOG_XP, A_PROBE   },   // ROW1
  { A_ZERO_Z, A_JOG_YM, A_JOG_ZM, A_FEED_DN,A_SEL_X, A_SEL_Y,  A_SEL_Z   },   // ROW2
};

bool     keyStable[3][7];
bool     keyRaw[3][7];
uint32_t keyChanged[3][7];

// ---------------- Fronta ťuknutí ----------------
struct Tap { uint8_t key; uint8_t mod; };
const uint8_t QSIZE = 255;          // 50 mm krok = 41 ťuknutí, rychlost až 12 – vejde se i víc nastavení za sebou
Tap      queue[QSIZE];
uint8_t  qHead = 0, qTail = 0;
bool     tapActive = false;
Tap      tapCur;
uint32_t tapStart = 0, lastTapEnd = 0;

bool qEmpty() { return qHead == qTail; }
bool qOverflow = false;              // ztracené ťuknutí => LED už nemusí odpovídat UGS
void qPush(uint8_t key, uint8_t mod) {
  uint8_t n = (qTail + 1) % QSIZE;
  if (n == qHead) { qOverflow = true; return; }   // plná fronta: zahodit a označit
  queue[qTail] = { key, mod };
  qTail = n;
}

// ---------------- Enkodér ----------------
volatile int32_t encEdges = 0;
volatile uint8_t encState = 0;
const int8_t QDEC[16] = { 0,-1, 1, 0,  1, 0, 0,-1,  -1, 0, 0, 1,  0, 1,-1, 0 };

void encIsr() {
  uint8_t s = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  encEdges += QDEC[(encState << 2) | s];
  encState = s;
}

bool     encSwStable = HIGH, encSwRaw = HIGH;
uint32_t encSwChanged = 0, encSwPressedAt = 0;

// ---------------- Stav ----------------
uint8_t  axis = 0;        // 0 = zamčeno, 1 X, 2 Y, 3 Z
uint8_t  heldKey = 0;     // držená jog klávesa (šipka)
Act      heldBy = A_NONE;
int8_t   pendingSteps = 0;
uint32_t lastEncTap = 0;

// ---------------- 74HC595 ----------------
// bit = 8*registr + výstup (QA=0 .. QH=7); QA (pin 15) se nepoužívá
// U4 (1. registr):  QB..QG  = krok XY 0.05 .. 100   -> bity 1-6
// U5 (2. registr):  QB..QG  = rychlost 10 .. MAX     -> bity 9-14
// U6 (3. registr):  QB..QE  = krok Z 0.05 .. 10      -> bity 17-20
//                   QF..QH  = osa X, Y, Z            -> bity 21-23
const uint8_t LEDB_XY   = 1;
const uint8_t LEDB_FEED = 9;
const uint8_t LEDB_Z    = 17;
const uint8_t LEDB_AXIS = 21;
uint32_t ledsShown = 0xFFFFFFFF;
uint32_t stopBlinkUntil = 0;                 // po STOP blikají všechny LED (5 s)
const uint16_t STOP_BLINK_MS = 5000;
uint32_t allLedsMask() {
  uint32_t v = 0;
  for (uint8_t i = 0; i < 6; i++) v |= (1UL << (LEDB_XY + i)) | (1UL << (LEDB_FEED + i));
  for (uint8_t i = 0; i < 4; i++) v |= 1UL << (LEDB_Z + i);
  for (uint8_t i = 0; i < 3; i++) v |= 1UL << (LEDB_AXIS + i);
  return v;
}

void writeLeds(uint32_t v) {
  if (v == ledsShown) return;
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 16) & 0xFF); // třetí 595
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 8) & 0xFF);  // druhý 595
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, v & 0xFF);         // první 595
  digitalWrite(PIN_595_LATCH, HIGH);
  ledsShown = v;
}

void updateLeds(uint32_t now) {
  if ((int32_t)(stopBlinkUntil - now) > 0) {           // STOP: všechny LED blikají
    writeLeds(((now / 200) % 2) ? allLedsMask() : 0);
    return;
  }
  bool blink = (now / 250) % 2;
  uint32_t v = 0;
  if (axis) v |= 1UL << (LEDB_AXIS + axis - 1);
  if (xySynced   || blink) v |= 1UL << (LEDB_XY + xyLevel);
  if (zSynced    || blink) v |= 1UL << (LEDB_Z + zLevel);
  if (feedSynced || blink) v |= 1UL << (LEDB_FEED + feedLevel);
  writeLeds(v);
}

// ---------------- Výstup kláves ----------------
void serviceTaps(uint32_t now) {
  if (tapActive) {
    if (now - tapStart >= TAP_HOLD_MS) {
      Keyboard.release(tapCur.key);
      if (tapCur.mod) Keyboard.release(tapCur.mod);
      tapActive = false;
      lastTapEnd = now;
    }
    return;
  }
  if (!qEmpty() && heldKey == 0 && now - lastTapEnd >= TAP_GAP_MS) {
    tapCur = queue[qHead];
    qHead = (qHead + 1) % QSIZE;
    if (tapCur.mod) Keyboard.press(tapCur.mod);
    Keyboard.press(tapCur.key);
    tapActive = true;
    tapStart = now;
  }
}

void releaseHeld() {
  if (!heldKey) return;
  Keyboard.release(heldKey);
  heldKey = 0;
  heldBy = A_NONE;
}

void stopAllMotion() {
  releaseHeld();
  pendingSteps = 0;
  noInterrupts(); encEdges = 0; interrupts();
}

// absolutní nastavení kroku XY
void setXyLevel(uint8_t lvl) {
  xyLevel = lvl; qOverflow = false;
  if (lvl == 0) {                                            // 0.05 = 0.01 + 4× 0.01
    qPush(KC_XY_STEP[0], CTRL);
    for (uint8_t i = 0; i < 4; i++) qPush(KC_XY_PLUS, CTRL);
  }
  else if (lvl <= 3) qPush(KC_XY_STEP[lvl], CTRL);          // 0.1 .. 10
  else {
    qPush(KC_XY_STEP[3], CTRL);                              // 10
    if (lvl == 4) for (uint8_t i = 0; i < 40; i++) qPush(KC_XY_PLUS, CTRL); // -> 50
    else qPush(KC_XY_MUL, CTRL);                             // -> 100
  }
  xySynced = !qOverflow;             // když se něco nevešlo, LED bliká = nesynchronizováno
}

void setZLevel(uint8_t lvl) {
  zLevel = lvl; qOverflow = false;
  qPush(KC_Z_STEP[lvl], CTRL);
  if (lvl == 0) for (uint8_t i = 0; i < 4; i++) qPush(KC_Z_PLUS, CTRL); // 0.01 -> 0.05
  zSynced = !qOverflow;
}

// absolutní nastavení rychlosti jogu
void setFeedLevel(uint8_t lvl) {
  feedLevel = lvl; qOverflow = false;
  for (uint8_t i = 0; i < FEED_RESET_TAPS; i++) qPush(KS_FEED_DIV, SHIFT); // -> 1
  uint8_t mul = (lvl <= 2) ? lvl + 1 : (lvl == 5 ? 4 : 3);   // 10/100/1000, MAX=10000
  for (uint8_t i = 0; i < mul; i++) qPush(KS_FEED_MUL, SHIFT);
  if (lvl == 3 || lvl == 4)                                    // 2000 / 3000
    for (uint8_t i = 0; i < lvl - 2; i++) qPush(KS_FEED_P1K, SHIFT);
  feedSynced = !qOverflow;
}

uint8_t stepDown(uint8_t lvl, bool synced)              { return synced && lvl > 0 ? lvl - 1 : lvl; }
uint8_t stepUp(uint8_t lvl, bool synced, uint8_t max)   { return synced && lvl < max - 1 ? lvl + 1 : lvl; }

uint8_t jogKeyFor(Act a) {
  switch (a) {
    case A_JOG_XM: return K_X_MINUS; case A_JOG_XP: return K_X_PLUS;
    case A_JOG_YP: return K_Y_PLUS;  case A_JOG_YM: return K_Y_MINUS;
    case A_JOG_ZP: return K_Z_PLUS;  case A_JOG_ZM: return K_Z_MINUS;
    default: return 0;
  }
}

void onPress(Act a) {
  bool unlocked = axis != 0;
  uint8_t jk = jogKeyFor(a);
  if (jk) {                                   // šipka: drží se jako fyzická klávesa
    if (unlocked && heldKey == 0 && !tapActive) {
      pendingSteps = 0;
      Keyboard.press(jk);
      heldKey = jk;
      heldBy = a;
    }
    return;
  }
  switch (a) {
    case A_HOLD:    releaseHeld(); qPush(K_HOLD, NOMOD);   break;
    case A_START:   qPush(K_START, NOMOD);                 break;
    case A_STOP:    stopAllMotion(); qPush(K_STOP, NOMOD); stopBlinkUntil = millis() + STOP_BLINK_MS; break;
    case A_UNLOCK:  qPush(K_UNLOCK, NOMOD);                break;
    case A_SEL_X:   axis = (axis == 1) ? 0 : 1; stopAllMotion(); break;
    case A_SEL_Y:   axis = (axis == 2) ? 0 : 2; stopAllMotion(); break;
    case A_SEL_Z:   axis = (axis == 3) ? 0 : 3; stopAllMotion(); break;
    case A_FEED_DN: setFeedLevel(stepDown(feedLevel, feedSynced)); break;
    case A_FEED_UP: setFeedLevel(stepUp(feedLevel, feedSynced, FEED_LEVELS)); break;
    case A_ZERO_X:  qPush(KS_ZERO_X, SHIFT);  break;
    case A_ZERO_Y:  qPush(KS_ZERO_Y, SHIFT);  break;
    case A_ZERO_Z:  qPush(KS_ZERO_Z, SHIFT);  break;
    case A_PROBE:   qPush(KS_PROBE, SHIFT);   break;
    case A_HOME:    qPush(KS_HOME, SHIFT);    break;
    case A_RTZ:     qPush(KS_RTZ, SHIFT);     break;
    default: break;
  }
}

void onRelease(Act a) {
  if (a == heldBy) releaseHeld();
}

void scanMatrix(uint32_t now) {
  for (uint8_t r = 0; r < 3; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(ROW_SETTLE_US);         // nechat ustálit filtr na sloupcích
    for (uint8_t c = 0; c < 7; c++) {
      bool pressed = digitalRead(COL_PINS[c]) == LOW;
      if (pressed != keyRaw[r][c]) { keyRaw[r][c] = pressed; keyChanged[r][c] = now; }
      if (pressed != keyStable[r][c] && now - keyChanged[r][c] >= DEBOUNCE_MS) {
        keyStable[r][c] = pressed;
        if (pressed) onPress(KEYMAP[r][c]); else onRelease(KEYMAP[r][c]);
      }
    }
    pinMode(ROW_PINS[r], INPUT);              // neaktivní řádek = vysoká impedance
  }
}

// ---------------- Setup ----------------
void setup() {
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_595_DATA, OUTPUT);
  pinMode(PIN_595_CLK, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);
  for (uint8_t r = 0; r < 3; r++) pinMode(ROW_PINS[r], INPUT);
  for (uint8_t c = 0; c < 7; c++) pinMode(COL_PINS[c], INPUT_PULLUP);   // vnitřní pull-up jako pojistka (na DPS B je navíc 10k)
  for (uint8_t r = 0; r < 3; r++)
    for (uint8_t c = 0; c < 7; c++) { keyStable[r][c] = keyRaw[r][c] = false; keyChanged[r][c] = 0; }

  writeLeds(0);
  pinMode(PIN_595_OE, OUTPUT);
  analogWrite(PIN_595_OE, 255 - LED_BRIGHT);   // OE je aktivní v LOW
  encState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encIsr, CHANGE);
  Keyboard.begin();
}

// ---------------- Loop ----------------
void loop() {
  uint32_t now = millis();

  if (!initSent && now >= INIT_DELAY_MS) {     // výchozí krok a rychlost do UGS
    initSent = true;
    setXyLevel(XY_START); setZLevel(Z_START); setFeedLevel(FEED_START);
  }

  scanMatrix(now);

  // tlačítko enkodéru: krátce větší krok, dlouze menší
  // zvolená osa Z mění krok Z, jinak krok XY
  bool r = digitalRead(PIN_ENC_SW);
  if (r != encSwRaw) { encSwRaw = r; encSwChanged = now; }
  if (r != encSwStable && now - encSwChanged >= DEBOUNCE_MS) {
    encSwStable = r;
    if (r == LOW) encSwPressedAt = now;
    else {
      bool longPress = now - encSwPressedAt >= LONG_PRESS_MS;
      if (axis == 3) setZLevel(longPress ? stepDown(zLevel, zSynced) : stepUp(zLevel, zSynced, Z_LEVELS));
      else           setXyLevel(longPress ? stepDown(xyLevel, xySynced) : stepUp(xyLevel, xySynced, XY_LEVELS));
    }
  }

  // enkodér: kroky zvolené osy
  int32_t edges;
  noInterrupts(); edges = encEdges; interrupts();
  int32_t detents = edges / EDGES_PER_DETENT;
  if (detents != 0) {
    noInterrupts(); encEdges -= detents * EDGES_PER_DETENT; interrupts();
    if (ENC_INVERT) detents = -detents;
    if (axis >= 1 && axis <= 3 && ENC_AXIS_FLIP[axis]) detents = -detents;   // směr zvlášť pro osu
    if (axis != 0 && heldKey == 0) {
      if ((detents > 0 && pendingSteps < 0) || (detents < 0 && pendingSteps > 0)) pendingSteps = 0;
      int32_t p = pendingSteps + detents;
      if (p >  MAX_PENDING) p =  MAX_PENDING;
      if (p < -MAX_PENDING) p = -MAX_PENDING;
      pendingSteps = (int8_t)p;
    }
  }
  if (pendingSteps != 0 && qEmpty() && !tapActive && heldKey == 0
      && now - lastEncTap >= ENC_GAP_MS) {
    bool plus = pendingSteps > 0;
    uint8_t k = 0;
    switch (axis) {
      case 1: k = plus ? K_X_PLUS : K_X_MINUS; break;
      case 2: k = plus ? K_Y_PLUS : K_Y_MINUS; break;
      case 3: k = plus ? K_Z_PLUS : K_Z_MINUS; break;
    }
    if (k) { qPush(k, NOMOD); pendingSteps += plus ? -1 : 1; lastEncTap = now; }
    else pendingSteps = 0;
  }

  serviceTaps(now);
  updateLeds(now);
}
