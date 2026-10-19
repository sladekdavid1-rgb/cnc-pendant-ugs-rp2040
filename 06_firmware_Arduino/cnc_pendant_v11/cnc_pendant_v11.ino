/*
  CNC pendant pro UGS (Universal Gcode Sender) + GRBL 1.1      verze 11.1
  RP2040 (Laskakit Ultimate Pi Pico / Pico) jako USB klávesnice (zkratky F13–F24 v UGS)

  Novinky proti verzi 10:
   1) ZRYCHLENÍ ENKODÉRU – při rychlém točení se krok v UGS dočasně zvětší o jeden
      stupeň (XY nejvýš na 10 mm, Z nejvýš na 1 mm), svítí i LED zvětšeného kroku.
      Po zastavení (0,4 s) se krok sám vrátí zpět.
   2) POJISTKA – START, HOME, RTZ, PROBE a NULA X/Y/Z se provedou až po PODRŽENÍ 0,5 s.
      Během držení rychle blikají LED os, po provedení krátce bliknou všechny LED.
      STOP, HOLD a UNLOCK reagují hned. STOP a HOLD navíc předběhnou frontu
      a zahodí čekající kroky jogu (stroj se zastaví hned, ne až po odeslání fronty).
   3) DIAGONÁLY – X+ a Y+ (atd.) zmáčknuté zároveň = šikmý pojezd (Ctrl+Shift+F13..F16).
   4) ENKODÉR BEZ ZVOLENÉ OSY mění rychlost (LED rychlosti se posouvá hned,
      do UGS se pošle 0,3 s po posledním cvaknutí).
   5) STMÍVÁNÍ LED – po 2 min bez dotyku se LED ztlumí, první dotek je rozsvítí
      (a zároveň normálně provede svou funkci).
   6) TESTOVACÍ REŽIM – drž STOP při připojení USB: nic se neposílá do PC,
      do Serial Monitoru (115200) se vypisují tlačítka a enkodér, držené tlačítko
      rozsvítí všechny LED, enkodér posouvá jednu LED. Konec = odpojit USB.
   Zvolená osa je potřeba jen pro pojezd (šipky, enkodér). Když se zmáčkne šipka bez zvolené
   osy, LED os 3× bliknou. HOME, RTZ, PROBE, NULA X/Y/Z, START, HOLD, STOP a UNLOCK fungují
   vždy (v11.1 – dřív HOME/RTZ/PROBE/NULA chtěly zvolenou osu, což byla chyba).

  Zkratky v UGS (nové jsou jen diagonály):
    F13/F14 X+/X-  F15/F16 Y+/Y-  F17/F18 Z+/Z-  F21 Pause  F22 Start  F23 Stop  F24 Unlock
    Shift+F13..F15 nula X/Y/Z  Shift+F16 Probe  Shift+F17 Home  Shift+F18 RTZ
    Shift+F19/F20 rychlost ×10/÷10  Shift+F21 rychlost +1000
    Ctrl+F13..F16 krok XY 0.01/0.1/1/10  Ctrl+F17 XY ×10  Ctrl+F18 XY +
    Ctrl+F19..F22 krok Z 0.01/0.1/1/10   Ctrl+F23 Z +
    Ctrl+Shift+F13 X+Y+  Ctrl+Shift+F14 X+Y-  Ctrl+Shift+F15 X-Y+  Ctrl+Shift+F16 X-Y-

  Arduino IDE: deska "Raspberry Pi Pico" (jádro arduino-pico, RP2040),
               Tools -> USB Stack: "Pico SDK"
*/

#include <Keyboard.h>

// ---------------- Piny ----------------
const uint8_t PIN_ENC_A  = 15;   // CLK
const uint8_t PIN_ENC_B  = 14;   // DT
const uint8_t PIN_ENC_SW = 13;   // SW (krátce = větší krok, dlouze = menší)

const uint8_t PIN_595_DATA  = 18; // SER
const uint8_t PIN_595_CLK   = 19; // SRCLK
const uint8_t PIN_595_LATCH = 20; // RCLK
const uint8_t PIN_595_OE    = 21; // OE (LOW = svítí) – PWM = jas

const uint8_t ROW_PINS[3] = { 2, 3, 4 };
const uint8_t COL_PINS[7] = { 5, 6, 7, 8, 9, 10, 11 };
const uint16_t ROW_SETTLE_US = 500;

// ---------------- Klávesy v UGS ----------------
const uint8_t NOMOD = 0, SHIFT = KEY_LEFT_SHIFT, CTRL = KEY_LEFT_CTRL;
const uint8_t CTRL_SHIFT = 0xFF;   // ve frontě: Ctrl+Shift naráz
const uint8_t K_X_PLUS  = KEY_F13, K_X_MINUS = KEY_F14;
const uint8_t K_Y_PLUS  = KEY_F15, K_Y_MINUS = KEY_F16;
const uint8_t K_Z_PLUS  = KEY_F17, K_Z_MINUS = KEY_F18;
const uint8_t K_HOLD      = KEY_F21;
const uint8_t K_START     = KEY_F22;
const uint8_t K_STOP      = KEY_F23;
const uint8_t K_UNLOCK    = KEY_F24;
const uint8_t KS_ZERO_X   = KEY_F13, KS_ZERO_Y = KEY_F14, KS_ZERO_Z = KEY_F15;
const uint8_t KS_PROBE    = KEY_F16, KS_HOME   = KEY_F17, KS_RTZ    = KEY_F18;
const uint8_t KS_FEED_MUL = KEY_F19;
const uint8_t KS_FEED_DIV = KEY_F20;
const uint8_t KS_FEED_P1K = KEY_F21;
const uint8_t KC_XY_STEP[4] = { KEY_F13, KEY_F14, KEY_F15, KEY_F16 }; // XY 0.01 / 0.1 / 1 / 10
const uint8_t KC_XY_MUL   = KEY_F17;
const uint8_t KC_XY_PLUS  = KEY_F18;
const uint8_t KC_Z_STEP[4]  = { KEY_F19, KEY_F20, KEY_F21, KEY_F22 }; // Z 0.01 / 0.1 / 1 / 10
const uint8_t KC_Z_PLUS   = KEY_F23;
// Ctrl+Shift + : diagonály
const uint8_t KD_XP_YP = KEY_F13, KD_XP_YM = KEY_F14, KD_XM_YP = KEY_F15, KD_XM_YM = KEY_F16;

// ---------------- Nastavení ----------------
const uint8_t  EDGES_PER_DETENT = 4;
const bool     ENC_INVERT       = true;    // směr otáčení enkodéru (true = obráceně – nastaveno podle hotového kusu; když jede opačně, dej false)
const uint16_t TAP_HOLD_MS      = 30;
const uint16_t TAP_GAP_MS       = 30;
const uint16_t ENC_GAP_MS       = 60;
const int8_t   MAX_PENDING      = 3;
const uint16_t DEBOUNCE_MS      = 20;
const uint16_t LONG_PRESS_MS    = 600;
const uint8_t  FEED_RESET_TAPS  = 6;

const uint16_t CONFIRM_MS       = 500;   // pojistka: jak dlouho držet START/HOME/RTZ/PROBE/NULA
const uint16_t CHORD_MS         = 60;    // okno pro zmáčknutí dvou šipek naráz (diagonála)
const uint16_t ENC_FAST_GAP_MS  = 60;    // cvaknutí rychleji než tohle = rychlé točení
const uint8_t  ENC_FAST_COUNT   = 4;     // tolik rychlých cvaknutí za sebou zapne zrychlení
const uint16_t ENC_BOOST_IDLE_MS= 400;   // po zastavení enkodéru se krok vrátí
const uint8_t  ENC_BOOST_MAX_XY = 3;     // zrychlení XY nejvýš na 10 mm
const uint8_t  ENC_BOOST_MAX_Z  = 2;     // zrychlení Z nejvýš na 1 mm
const uint16_t ENC_FEED_SEND_MS = 300;   // enkodér bez osy: poslat rychlost po této pauze

const uint8_t  LED_BRIGHT       = 255;   // jas LED 0..255
const uint8_t  LED_DIM          = 20;    // jas po ztlumení
const uint32_t LED_DIM_AFTER_MS = 120000UL;  // ztlumit po 2 min bez dotyku (0 = nikdy)

// Krok XY:  0..5 = 0.05 / 0.1 / 1 / 10 / 50 / 100 mm
// Krok Z:   0..3 = 0.05 / 0.1 / 1 / 10 mm
// Rychlost: 0..5 = 10 / 100 / 1000 / 2000 / 3000 / MAX mm/min
const uint8_t XY_LEVELS = 6, Z_LEVELS = 4, FEED_LEVELS = 6;
const uint8_t  XY_START = 3;              // krok XY 10 mm
const uint8_t  Z_START  = 2;              // krok Z 1 mm
const uint8_t  FEED_START = 2;            // rychlost 1000 mm/min
const uint16_t INIT_DELAY_MS = 2500;
uint8_t xyLevel = XY_START, zLevel = Z_START, feedLevel = FEED_START;
bool    xySynced = true, zSynced = true, feedSynced = true;
bool    initSent = false;

// ---------------- Matice tlačítek ----------------
enum Act : uint8_t {
  A_NONE, A_JOG_XM, A_JOG_XP, A_JOG_YP, A_JOG_YM, A_JOG_ZP, A_JOG_ZM,
  A_HOLD, A_START, A_STOP, A_UNLOCK, A_SEL_X, A_SEL_Y, A_SEL_Z,
  A_FEED_DN, A_FEED_UP, A_ZERO_X, A_ZERO_Y, A_ZERO_Z, A_PROBE, A_HOME, A_RTZ
};
const Act KEYMAP[3][7] = {
  { A_UNLOCK, A_START,  A_HOLD,   A_STOP,  A_HOME,   A_RTZ,    A_ZERO_X  },
  { A_JOG_YP, A_JOG_ZP, A_FEED_UP,A_ZERO_Y,A_JOG_XM, A_JOG_XP, A_PROBE   },
  { A_ZERO_Z, A_JOG_YM, A_JOG_ZM, A_FEED_DN,A_SEL_X, A_SEL_Y,  A_SEL_Z   },
};
const char* NAME[3][7] = {
  { "UNLOCK", "START", "HOLD", "STOP", "HOME", "RTZ", "NULA X" },
  { "Y+", "Z+", "F+", "NULA Y", "X-", "X+", "PROBE" },
  { "NULA Z", "Y-", "Z-", "F-", "OSA X", "OSA Y", "OSA Z" },
};
const uint8_t STOP_ROW = 0, STOP_COL = 3;   // drž při připojení = testovací režim

bool     keyStable[3][7];
bool     keyRaw[3][7];
uint32_t keyChanged[3][7];
uint32_t keyPressedAt[3][7];
bool     keyFired[3][7];

// ---------------- Fronta ťuknutí ----------------
struct Tap { uint8_t key; uint8_t mod; };
const uint8_t QSIZE = 255;
Tap      queue[QSIZE];
uint8_t  qHead = 0, qTail = 0;
bool     tapActive = false;
Tap      tapCur;
uint32_t tapStart = 0, lastTapEnd = 0;

bool qEmpty() { return qHead == qTail; }
bool qOverflow = false;
void qPush(uint8_t key, uint8_t mod) {
  uint8_t n = (qTail + 1) % QSIZE;
  if (n == qHead) { qOverflow = true; return; }
  queue[qTail] = { key, mod };
  qTail = n;
}
void qPushFront(uint8_t key, uint8_t mod) {          // STOP / HOLD předběhnou frontu
  uint8_t n = (qHead + QSIZE - 1) % QSIZE;
  if (n == qTail) { qOverflow = true; return; }
  qHead = n;
  queue[qHead] = { key, mod };
}
bool isJogTap(const Tap& t) {
  return (t.mod == NOMOD && t.key >= KEY_F13 && t.key <= KEY_F18) ||
         (t.mod == CTRL_SHIFT && t.key >= KEY_F13 && t.key <= KEY_F16);   // i diagonály
}
void qDropJogs() {                                    // zahodit čekající kroky jogu, nastavení nechat
  Tap tmp[QSIZE]; uint8_t n = 0;
  while (!qEmpty()) { Tap t = queue[qHead]; qHead = (qHead + 1) % QSIZE; if (!isJogTap(t)) tmp[n++] = t; }
  qHead = qTail = 0;
  for (uint8_t i = 0; i < n; i++) qPush(tmp[i].key, tmp[i].mod);
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
uint32_t lastDetentAt = 0;
uint8_t  fastCount = 0;
bool     boostActive = false;
uint8_t  boostAxis = 0, boostLevel = 0;
bool     feedDirty = false;
uint32_t feedDirtyAt = 0;

// ---------------- Stav ----------------
uint8_t  axis = 0;        // 0 = zamčeno, 1 X, 2 Y, 3 Z
uint8_t  heldKey = 0;     // držená jog klávesa
bool     heldDiag = false;// drží se i Ctrl+Shift (diagonála)
Act      heldBy = A_NONE, heldBy2 = A_NONE;
Act      jogPend = A_NONE;// šipka čeká na start (okno pro druhou šipku / dokončení fronty)
Act      jogPend2 = A_NONE;// druhá šipka = diagonála
uint32_t jogPendAt = 0;
int8_t   pendingSteps = 0;
uint32_t lastEncTap = 0;
uint32_t lastActivity = 0;
bool     testMode = false;
int      testLed = 0;

// ---------------- 74HC595 ----------------
const uint8_t LEDB_XY   = 1;
const uint8_t LEDB_FEED = 9;
const uint8_t LEDB_Z    = 17;
const uint8_t LEDB_AXIS = 21;
const uint8_t LEDBIT[19] = { 21, 22, 23,  1, 2, 3, 4, 5, 6,  17, 18, 19, 20,  9, 10, 11, 12, 13, 14 };
const uint32_t AXIS_MASK = 7UL << LEDB_AXIS;
uint32_t ledsShown = 0xFFFFFFFF;
uint32_t stopBlinkUntil = 0;
const uint16_t STOP_BLINK_MS = 5000;
uint32_t flashUntil = 0;     // po potvrzení podržením: krátce všechny LED
uint32_t hintUntil = 0;      // chybí zvolená osa: LED os blikají
int16_t  brightNow = -1;

uint32_t allLedsMask() {
  uint32_t v = 0;
  for (uint8_t i = 0; i < 19; i++) v |= 1UL << LEDBIT[i];
  return v;
}
void writeLeds(uint32_t v) {
  if (v == ledsShown) return;
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 16) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 8) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, v & 0xFF);
  digitalWrite(PIN_595_LATCH, HIGH);
  ledsShown = v;
}
void setBright(uint8_t b) {
  if (b == brightNow) return;
  analogWrite(PIN_595_OE, 255 - b);      // OE je aktivní v LOW
  brightNow = b;
}
void activity(uint32_t now) { lastActivity = now; }

bool isConfirm(Act a) {
  return a == A_START || a == A_HOME || a == A_RTZ || a == A_PROBE ||
         a == A_ZERO_X || a == A_ZERO_Y || a == A_ZERO_Z;
}
bool confirmWaiting() {                  // drží se tlačítko s pojistkou, ještě neproběhlo
  for (uint8_t r = 0; r < 3; r++)
    for (uint8_t c = 0; c < 7; c++)
      if (keyStable[r][c] && !keyFired[r][c] && isConfirm(KEYMAP[r][c])) return true;
  return false;
}

void updateLeds(uint32_t now) {
  bool awake = LED_DIM_AFTER_MS == 0 || now - lastActivity < LED_DIM_AFTER_MS;
  setBright(awake ? LED_BRIGHT : LED_DIM);
  if ((int32_t)(stopBlinkUntil - now) > 0) { writeLeds(((now / 200) % 2) ? allLedsMask() : 0); return; }
  if ((int32_t)(flashUntil - now) > 0)     { writeLeds(allLedsMask()); return; }
  bool blink = (now / 250) % 2;
  uint32_t v = 0;
  if (axis) v |= 1UL << (LEDB_AXIS + axis - 1);
  if (xySynced   || blink) v |= 1UL << (LEDB_XY + xyLevel);
  if (zSynced    || blink) v |= 1UL << (LEDB_Z + zLevel);
  if (feedSynced || blink) v |= 1UL << (LEDB_FEED + feedLevel);
  if (boostActive) v |= 1UL << ((boostAxis == 3 ? LEDB_Z : LEDB_XY) + boostLevel);
  if ((int32_t)(hintUntil - now) > 0) v = (v & ~AXIS_MASK) | (((now / 150) % 2) ? AXIS_MASK : 0);
  else if (confirmWaiting())          v = (v & ~AXIS_MASK) | (((now / 80) % 2) ? AXIS_MASK : 0);
  writeLeds(v);
}

// ---------------- Výstup kláves ----------------
void modsPress(uint8_t m) {
  if (m == CTRL_SHIFT) { Keyboard.press(SHIFT); Keyboard.press(CTRL); }
  else if (m) Keyboard.press(m);
}
void modsRelease(uint8_t m) {
  if (m == CTRL_SHIFT) { Keyboard.release(CTRL); Keyboard.release(SHIFT); }
  else if (m) Keyboard.release(m);
}
void serviceTaps(uint32_t now) {
  if (tapActive) {
    if (now - tapStart >= TAP_HOLD_MS) {
      Keyboard.release(tapCur.key);
      modsRelease(tapCur.mod);
      tapActive = false;
      lastTapEnd = now;
    }
    return;
  }
  if (!qEmpty() && heldKey == 0 && now - lastTapEnd >= TAP_GAP_MS) {
    tapCur = queue[qHead];
    qHead = (qHead + 1) % QSIZE;
    modsPress(tapCur.mod);
    Keyboard.press(tapCur.key);
    tapActive = true;
    tapStart = now;
  }
}

void releaseHeld() {
  if (!heldKey) return;
  Keyboard.release(heldKey);
  if (heldDiag) { Keyboard.release(CTRL); Keyboard.release(SHIFT); }
  heldKey = 0; heldDiag = false;
  heldBy = heldBy2 = A_NONE;
}
void startJog(uint32_t now);

// ---- odeslání kroku / rychlosti (jen do fronty, proměnné nemění) ----
void pushXy(uint8_t lvl) {
  if (lvl == 0) { qPush(KC_XY_STEP[0], CTRL); for (uint8_t i = 0; i < 4; i++) qPush(KC_XY_PLUS, CTRL); }
  else if (lvl <= 3) qPush(KC_XY_STEP[lvl], CTRL);
  else {
    qPush(KC_XY_STEP[3], CTRL);
    if (lvl == 4) for (uint8_t i = 0; i < 40; i++) qPush(KC_XY_PLUS, CTRL);
    else qPush(KC_XY_MUL, CTRL);
  }
}
void pushZ(uint8_t lvl) {
  qPush(KC_Z_STEP[lvl], CTRL);
  if (lvl == 0) for (uint8_t i = 0; i < 4; i++) qPush(KC_Z_PLUS, CTRL);
}
void setXyLevel(uint8_t lvl) {
  if (boostActive && boostAxis != 3) boostActive = false;   // absolutní nastavení ruší zrychlení
  xyLevel = lvl; qOverflow = false; pushXy(lvl); xySynced = !qOverflow;
}
void setZLevel(uint8_t lvl) {
  if (boostActive && boostAxis == 3) boostActive = false;
  zLevel = lvl; qOverflow = false; pushZ(lvl); zSynced = !qOverflow;
}
void setFeedLevel(uint8_t lvl) {
  feedLevel = lvl; qOverflow = false; feedDirty = false;
  for (uint8_t i = 0; i < FEED_RESET_TAPS; i++) qPush(KS_FEED_DIV, SHIFT);
  uint8_t mul = (lvl <= 2) ? lvl + 1 : (lvl == 5 ? 4 : 3);
  for (uint8_t i = 0; i < mul; i++) qPush(KS_FEED_MUL, SHIFT);
  if (lvl == 3 || lvl == 4) for (uint8_t i = 0; i < lvl - 2; i++) qPush(KS_FEED_P1K, SHIFT);
  feedSynced = !qOverflow;
}
void endBoost() {                       // vrátit krok po zrychlení
  if (!boostActive) return;
  boostActive = false;
  if (boostAxis == 3) pushZ(zLevel); else pushXy(xyLevel);
}

void stopAllMotion() {
  releaseHeld();
  jogPend = jogPend2 = A_NONE;
  pendingSteps = 0;
  noInterrupts(); encEdges = 0; interrupts();
  qDropJogs();
  endBoost();
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
bool isX(Act a) { return a == A_JOG_XP || a == A_JOG_XM; }
bool isY(Act a) { return a == A_JOG_YP || a == A_JOG_YM; }
uint8_t diagKey(Act a, Act b) {          // a, b = jedna šipka X a jedna Y (v libovolném pořadí)
  Act x = isX(a) ? a : b, y = isY(a) ? a : b;
  if (x == A_JOG_XP) return y == A_JOG_YP ? KD_XP_YP : KD_XP_YM;
  return y == A_JOG_YP ? KD_XM_YP : KD_XM_YM;
}
// rozjet čekající šipku (nebo diagonálu) jako drženou klávesu
void startJog(uint32_t now) {
  pendingSteps = 0;
  if (jogPend2 != A_NONE) {
    uint8_t k = diagKey(jogPend, jogPend2);
    Keyboard.press(SHIFT); Keyboard.press(CTRL); Keyboard.press(k);
    heldKey = k; heldDiag = true; heldBy = jogPend; heldBy2 = jogPend2;
  } else {
    Keyboard.press(jogKeyFor(jogPend));
    heldKey = jogKeyFor(jogPend); heldDiag = false; heldBy = jogPend; heldBy2 = A_NONE;
  }
  jogPend = jogPend2 = A_NONE;
}

// provedení akce s pojistkou (po podržení)
void doConfirmed(Act a) {
  switch (a) {
    case A_START:  qPush(K_START, NOMOD);    break;
    case A_ZERO_X: qPush(KS_ZERO_X, SHIFT);  break;
    case A_ZERO_Y: qPush(KS_ZERO_Y, SHIFT);  break;
    case A_ZERO_Z: qPush(KS_ZERO_Z, SHIFT);  break;
    case A_PROBE:  qPush(KS_PROBE, SHIFT);   break;
    case A_HOME:   qPush(KS_HOME, SHIFT);    break;
    case A_RTZ:    qPush(KS_RTZ, SHIFT);     break;
    default: return;
  }
  flashUntil = millis() + 150;
}

void onPress(Act a, uint32_t now) {
  bool unlocked = axis != 0;
  if (jogKeyFor(a)) {
    if (!unlocked) { hintUntil = now + 900; return; }
    if (heldKey != 0) return;                       // už se jede
    if (jogPend == A_NONE) {                        // první šipka: počkat na případnou druhou
      endBoost();
      jogPend = a; jogPendAt = now;
    } else if (jogPend2 == A_NONE &&
               ((isX(jogPend) && isY(a)) || (isY(jogPend) && isX(a)))) {
      jogPend2 = a;                                 // X + Y = diagonála
    }
    return;
  }
  if (isConfirm(a)) return;                         // provede se až po podržení (loop), osa není potřeba
  switch (a) {
    case A_HOLD:    releaseHeld(); jogPend = jogPend2 = A_NONE; qDropJogs(); qPushFront(K_HOLD, NOMOD); break;
    case A_STOP:    stopAllMotion(); qPushFront(K_STOP, NOMOD); stopBlinkUntil = now + STOP_BLINK_MS; break;
    case A_UNLOCK:  qPush(K_UNLOCK, NOMOD); break;
    case A_SEL_X:   stopAllMotion(); axis = (axis == 1) ? 0 : 1; break;
    case A_SEL_Y:   stopAllMotion(); axis = (axis == 2) ? 0 : 2; break;
    case A_SEL_Z:   stopAllMotion(); axis = (axis == 3) ? 0 : 3; break;
    case A_FEED_DN: setFeedLevel(stepDown(feedLevel, feedSynced)); break;
    case A_FEED_UP: setFeedLevel(stepUp(feedLevel, feedSynced, FEED_LEVELS)); break;
    default: break;
  }
}

void onRelease(Act a) {
  if (jogPend != A_NONE && (a == jogPend || a == jogPend2)) {   // krátké ťuknutí: jeden krok
    if (jogPend2 != A_NONE) qPush(diagKey(jogPend, jogPend2), CTRL_SHIFT);
    else                    qPush(jogKeyFor(jogPend), NOMOD);
    jogPend = jogPend2 = A_NONE;
    return;
  }
  if (heldKey && (a == heldBy || a == heldBy2)) releaseHeld();
}

// ---------------- Testovací režim ----------------
void testKey(uint8_t r, uint8_t c, bool pressed) {
  if (!pressed) return;
  uint8_t n = r * 7 + c + 1;
  Serial.print("SW"); if (n < 10) Serial.print('0'); Serial.print(n);
  Serial.print("  "); Serial.print(NAME[r][c]);
  Serial.print("   R"); Serial.print(r); Serial.print(" C"); Serial.println(c);
}

void scanMatrix(uint32_t now) {
  for (uint8_t r = 0; r < 3; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(ROW_SETTLE_US);
    for (uint8_t c = 0; c < 7; c++) {
      bool pressed = digitalRead(COL_PINS[c]) == LOW;
      if (pressed != keyRaw[r][c]) { keyRaw[r][c] = pressed; keyChanged[r][c] = now; }
      if (pressed != keyStable[r][c] && now - keyChanged[r][c] >= DEBOUNCE_MS) {
        keyStable[r][c] = pressed;
        activity(now);
        if (testMode) { testKey(r, c, pressed); continue; }
        if (pressed) { keyPressedAt[r][c] = now; keyFired[r][c] = false; onPress(KEYMAP[r][c], now); }
        else onRelease(KEYMAP[r][c]);
      }
    }
    pinMode(ROW_PINS[r], INPUT);
  }
}

void checkConfirm(uint32_t now) {
  for (uint8_t r = 0; r < 3; r++)
    for (uint8_t c = 0; c < 7; c++) {
      Act a = KEYMAP[r][c];
      if (!keyStable[r][c] || keyFired[r][c] || !isConfirm(a)) continue;
      if (now - keyPressedAt[r][c] >= CONFIRM_MS) { keyFired[r][c] = true; activity(now); doConfirmed(a); }
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
  for (uint8_t c = 0; c < 7; c++) pinMode(COL_PINS[c], INPUT_PULLUP);
  for (uint8_t r = 0; r < 3; r++)
    for (uint8_t c = 0; c < 7; c++) {
      keyStable[r][c] = keyRaw[r][c] = keyFired[r][c] = false; keyChanged[r][c] = keyPressedAt[r][c] = 0;
    }

  // testovací režim: STOP držený při připojení
  pinMode(ROW_PINS[STOP_ROW], OUTPUT); digitalWrite(ROW_PINS[STOP_ROW], LOW);
  delay(2);
  testMode = digitalRead(COL_PINS[STOP_COL]) == LOW;
  pinMode(ROW_PINS[STOP_ROW], INPUT);
  if (testMode) keyStable[STOP_ROW][STOP_COL] = keyRaw[STOP_ROW][STOP_COL] = true;

  writeLeds(0);
  pinMode(PIN_595_OE, OUTPUT);
  setBright(LED_BRIGHT);
  encState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encIsr, CHANGE);
  Serial.begin(115200);
  if (!testMode) Keyboard.begin();
}

// ---------------- Loop testovacího režimu ----------------
void testLoop(uint32_t now) {
  static bool banner = false;
  if (Serial && !banner) {
    banner = true;
    Serial.println("=== CNC pendant v11 - TESTOVACI REZIM (do PC se nic neposila) ===");
    Serial.println("Mackej tlacitka, toc enkoderem. Konec = odpojit USB.");
  }
  scanMatrix(now);
  bool r = digitalRead(PIN_ENC_SW);
  if (r != encSwRaw) { encSwRaw = r; encSwChanged = now; }
  if (r != encSwStable && now - encSwChanged >= DEBOUNCE_MS) {
    encSwStable = r;
    if (r == LOW) Serial.println("ENKODER TLACITKO");
  }
  int32_t e; noInterrupts(); e = encEdges; interrupts();
  int32_t d = e / EDGES_PER_DETENT;
  if (d != 0) {
    noInterrupts(); encEdges -= d * EDGES_PER_DETENT; interrupts();
    if (ENC_INVERT) d = -d;
    testLed = ((testLed + d) % 19 + 19) % 19;
    Serial.print("ENKODER "); Serial.print(d > 0 ? "+" : ""); Serial.print(d);
    Serial.print("   LED "); Serial.println(testLed + 1);
  }
  bool any = false;
  for (uint8_t rr = 0; rr < 3; rr++) for (uint8_t c = 0; c < 7; c++) any |= keyStable[rr][c];
  setBright(LED_BRIGHT);
  writeLeds(any ? allLedsMask() : (1UL << LEDBIT[testLed]));
}

// ---------------- Loop ----------------
void loop() {
  uint32_t now = millis();
  if (testMode) { testLoop(now); return; }

  if (!initSent && now >= INIT_DELAY_MS) {
    initSent = true;
    setXyLevel(XY_START); setZLevel(Z_START); setFeedLevel(FEED_START);
  }

  scanMatrix(now);
  checkConfirm(now);

  // šipka (diagonála) se rozjede po okně CHORD_MS, až je fronta prázdná
  // (nejdřív se v UGS dokončí nastavení kroku / návrat po zrychlení)
  if (jogPend != A_NONE && heldKey == 0 && !tapActive && qEmpty() &&
      (jogPend2 != A_NONE || now - jogPendAt >= CHORD_MS)) startJog(now);

  // tlačítko enkodéru: krátce větší krok, dlouze menší (osa Z = krok Z, jinak XY)
  bool r = digitalRead(PIN_ENC_SW);
  if (r != encSwRaw) { encSwRaw = r; encSwChanged = now; }
  if (r != encSwStable && now - encSwChanged >= DEBOUNCE_MS) {
    encSwStable = r;
    activity(now);
    if (r == LOW) encSwPressedAt = now;
    else {
      bool longPress = now - encSwPressedAt >= LONG_PRESS_MS;
      if (axis == 3) setZLevel(longPress ? stepDown(zLevel, zSynced) : stepUp(zLevel, zSynced, Z_LEVELS));
      else           setXyLevel(longPress ? stepDown(xyLevel, xySynced) : stepUp(xyLevel, xySynced, XY_LEVELS));
    }
  }

  // enkodér
  int32_t edges;
  noInterrupts(); edges = encEdges; interrupts();
  int32_t detents = edges / EDGES_PER_DETENT;
  if (detents != 0) {
    noInterrupts(); encEdges -= detents * EDGES_PER_DETENT; interrupts();
    if (ENC_INVERT) detents = -detents;
    activity(now);
    uint8_t ad = detents > 0 ? detents : -detents;
    if (now - lastDetentAt < ENC_FAST_GAP_MS) { uint16_t f = fastCount + ad; fastCount = f > 255 ? 255 : f; }
    else fastCount = ad;
    lastDetentAt = now;

    if (axis == 0) {                                  // bez osy: rychlost
      int nl = (int)feedLevel + detents;
      if (nl < 0) nl = 0;
      if (nl > FEED_LEVELS - 1) nl = FEED_LEVELS - 1;
      if (nl != feedLevel) { feedLevel = nl; feedDirty = true; feedDirtyAt = now; }
    } else if (heldKey == 0 && jogPend == A_NONE) {
      // zrychlení: dočasně o stupeň větší krok
      if (!boostActive && fastCount >= ENC_FAST_COUNT) {
        if (axis == 3 && zSynced && zLevel < ENC_BOOST_MAX_Z) {
          boostActive = true; boostAxis = 3; boostLevel = zLevel + 1; pushZ(boostLevel);
        } else if (axis != 3 && xySynced && xyLevel < ENC_BOOST_MAX_XY) {
          boostActive = true; boostAxis = axis; boostLevel = xyLevel + 1; pushXy(boostLevel);
        }
      }
      if ((detents > 0 && pendingSteps < 0) || (detents < 0 && pendingSteps > 0)) pendingSteps = 0;
      int32_t p = pendingSteps + detents;
      if (p >  MAX_PENDING) p =  MAX_PENDING;
      if (p < -MAX_PENDING) p = -MAX_PENDING;
      pendingSteps = (int8_t)p;
    }
  }
  if (feedDirty && now - feedDirtyAt >= ENC_FEED_SEND_MS) setFeedLevel(feedLevel);

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

  // konec zrychlení: enkodér stojí a všechny kroky odešly
  if (boostActive && pendingSteps == 0 && qEmpty() && !tapActive
      && now - lastDetentAt >= ENC_BOOST_IDLE_MS) endBoost();

  serviceTaps(now);
  updateLeds(now);
}
