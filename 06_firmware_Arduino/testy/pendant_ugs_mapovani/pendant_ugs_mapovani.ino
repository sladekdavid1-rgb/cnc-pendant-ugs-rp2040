/*
  CNC pendant – POMOCNÍK PRO NASTAVENÍ ZKRATEK V UGS
  Každé tlačítko pošle právě JEDNU klávesovou zkratku, kterou pak hlavní program
  (cnc_pendant.ino) používá. V UGS: Tools -> Options -> Keymap, klikni do pole
  zkratky u akce a zmáčkni tlačítko na pendantu – UGS si zkratku zapíše.

  Základní vrstva (stačí zmáčknout):
    X+ F13   X- F14   Y+ F15   Y- F16   Z+ F17   Z- F18
    HOLD F21   START F22   STOP F23   UNLOCK F24
    NULA X  Shift+F13   NULA Y Shift+F14   NULA Z Shift+F15   PROBE Shift+F16
    HOME    Shift+F17   RTZ    Shift+F18
    F+      Shift+F19 (rychlost ×10)   F- Shift+F20 (rychlost ÷10)   OSA X Shift+F21 (rychlost +1000)
  Druhá vrstva (DRŽ tlačítko OSA Z a zmáčkni):
    NULA X  Ctrl+F13 (krok XY 0.01)   NULA Y Ctrl+F14 (XY 0.1)   NULA Z Ctrl+F15 (XY 1)
    PROBE   Ctrl+F16 (XY 10)          HOME   Ctrl+F17 (XY ×10)   RTZ    Ctrl+F18 (XY +)
    F+      Ctrl+F19 (krok Z 0.01)    F-     Ctrl+F20 (Z 0.1)    HOLD   Ctrl+F21 (Z 1)
    START   Ctrl+F22 (Z 10)           STOP   Ctrl+F23 (Z +)
  Co se poslalo, se vypisuje do Serial Monitoru (115200). Nic jiného se nepíše.
  Po nastavení UGS nahraj zpět cnc_pendant.ino.
*/
#include <Keyboard.h>

const uint8_t ROW_PINS[3] = { 2, 3, 4 };
const uint8_t COL_PINS[7] = { 5, 6, 7, 8, 9, 10, 11 };
const uint8_t PIN_595_DATA = 18, PIN_595_CLK = 19, PIN_595_LATCH = 20, PIN_595_OE = 21;
const uint16_t ROW_SETTLE_US = 500, DEBOUNCE_MS = 20, HOLD_MS = 60;

const uint8_t S = KEY_LEFT_SHIFT, C = KEY_LEFT_CTRL;
struct Key { uint8_t mod, key; const char* txt; };
// [řádek][sloupec] stejně jako v hlavním programu
const char* NAME[3][7] = {
  { "UNLOCK", "START", "HOLD", "STOP", "HOME", "RTZ", "NULA X" },
  { "Y+", "Z+", "F+", "NULA Y", "X-", "X+", "PROBE" },
  { "NULA Z", "Y-", "Z-", "F-", "OSA X", "OSA Y", "OSA Z" },
};
const Key BASE[3][7] = {
  { {0, KEY_F24, "F24  Unlock"}, {0, KEY_F22, "F22  Start"}, {0, KEY_F21, "F21  Pause/Resume"}, {0, KEY_F23, "F23  Stop"},
    {S, KEY_F17, "Shift+F17  Home"}, {S, KEY_F18, "Shift+F18  Return to zero"}, {S, KEY_F13, "Shift+F13  Reset X zero"} },
  { {0, KEY_F15, "F15  Jog Y+"}, {0, KEY_F17, "F17  Jog Z+"}, {S, KEY_F19, "Shift+F19  Jog feed x10"}, {S, KEY_F14, "Shift+F14  Reset Y zero"},
    {0, KEY_F14, "F14  Jog X-"}, {0, KEY_F13, "F13  Jog X+"}, {S, KEY_F16, "Shift+F16  Probe"} },
  { {S, KEY_F15, "Shift+F15  Reset Z zero"}, {0, KEY_F16, "F16  Jog Y-"}, {0, KEY_F18, "F18  Jog Z-"}, {S, KEY_F20, "Shift+F20  Jog feed /10"},
    {S, KEY_F21, "Shift+F21  Jog feed +1000"}, {0, 0, "(nic)"}, {0, 0, "(vrstva Ctrl - drz)"} },
};
const Key CTRL[3][7] = {
  { {0, 0, "(nic)"}, {C, KEY_F22, "Ctrl+F22  krok Z 10"}, {C, KEY_F21, "Ctrl+F21  krok Z 1"}, {C, KEY_F23, "Ctrl+F23  krok Z +"},
    {C, KEY_F17, "Ctrl+F17  krok XY x10"}, {C, KEY_F18, "Ctrl+F18  krok XY +"}, {C, KEY_F13, "Ctrl+F13  krok XY 0.01"} },
  { {0, 0, "(nic)"}, {0, 0, "(nic)"}, {C, KEY_F19, "Ctrl+F19  krok Z 0.01"}, {C, KEY_F14, "Ctrl+F14  krok XY 0.1"},
    {0, 0, "(nic)"}, {0, 0, "(nic)"}, {C, KEY_F16, "Ctrl+F16  krok XY 10"} },
  { {C, KEY_F15, "Ctrl+F15  krok XY 1"}, {0, 0, "(nic)"}, {0, 0, "(nic)"}, {C, KEY_F20, "Ctrl+F20  krok Z 0.1"},
    {0, 0, "(nic)"}, {0, 0, "(nic)"}, {0, 0, "(vrstva Ctrl - drz)"} },
};

bool keyStable[3][7], keyRaw[3][7];
uint32_t keyChanged[3][7];

void send(const Key& k, const char* name) {
  Serial.print("["); Serial.print(name); Serial.print("]  "); Serial.println(k.txt);
  if (!k.key) return;
  if (k.mod) Keyboard.press(k.mod);
  Keyboard.press(k.key); delay(HOLD_MS);
  Keyboard.release(k.key);
  if (k.mod) Keyboard.release(k.mod);
}

void writeLeds(uint32_t v) {
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 16) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, (v >> 8) & 0xFF);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, v & 0xFF);
  digitalWrite(PIN_595_LATCH, HIGH);
}

void setup() {
  Serial.begin(115200);
  for (uint8_t r = 0; r < 3; r++) pinMode(ROW_PINS[r], INPUT);
  for (uint8_t c = 0; c < 7; c++) pinMode(COL_PINS[c], INPUT_PULLUP);
  pinMode(PIN_595_DATA, OUTPUT); pinMode(PIN_595_CLK, OUTPUT); pinMode(PIN_595_LATCH, OUTPUT);
  writeLeds(0); pinMode(PIN_595_OE, OUTPUT); digitalWrite(PIN_595_OE, LOW);
  Keyboard.begin();
}

void loop() {
  uint32_t now = millis();
  bool layer = keyStable[2][6];                 // drženo OSA Z = vrstva Ctrl
  writeLeds(layer ? (1UL << 23) : (1UL << 21)); // svítí LED osy Z = vrstva Ctrl, osa X = základní
  for (uint8_t r = 0; r < 3; r++) {
    pinMode(ROW_PINS[r], OUTPUT); digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(ROW_SETTLE_US);
    for (uint8_t c = 0; c < 7; c++) {
      bool p = digitalRead(COL_PINS[c]) == LOW;
      if (p != keyRaw[r][c]) { keyRaw[r][c] = p; keyChanged[r][c] = now; }
      if (p != keyStable[r][c] && now - keyChanged[r][c] >= DEBOUNCE_MS) {
        keyStable[r][c] = p;
        if (p && !(r == 2 && c == 6)) send(layer ? CTRL[r][c] : BASE[r][c], NAME[r][c]);
      }
    }
    pinMode(ROW_PINS[r], INPUT);
  }
}
