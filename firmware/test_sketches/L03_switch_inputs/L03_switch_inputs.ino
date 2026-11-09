/*
 * L03_switch_inputs.ino  -  Lesson 03: door reed switch + tamper switch (DET, Board 2)
 * Reads two switch inputs with pull-ups, debounces them in software (50 ms)
 * and prints every state change with a timestamp and the number of raw edges
 * (bounces) seen during that change. Serial Monitor 115200.
 *
 * Wiring (see docs/learning/03_switch_inputs.md):
 *   GPIO26 <- reed switch to GND, 10k pull-up to 3V3, 100 nF to GND
 *   GPIO25 <- tamper microswitch (COM to GND, NO to pin), 10k pull-up, 100 nF
 */
const int PIN_REED   = 26;   // LOW = magnet near = door CLOSED
const int PIN_TAMPER = 25;   // LOW = lid presses switch = lid CLOSED
const uint32_t DEBOUNCE_MS = 50;

struct DebouncedInput {
  int      pin;
  const char *name;
  const char *lowText;     // meaning of LOW (switch closed)
  const char *highText;    // meaning of HIGH (switch open)
  int      raw;            // last raw reading
  int      stable;         // debounced state
  uint32_t lastEdgeMs;     // time of last raw change
  uint16_t edges;          // raw edges since last stable change
};

DebouncedInput reed   = {PIN_REED,   "DOOR", "CLOSED", "OPEN",              HIGH, HIGH, 0, 0};
DebouncedInput tamper = {PIN_TAMPER, "LID ", "CLOSED", "OPEN -> TAMPER",    HIGH, HIGH, 0, 0};

void initInput(DebouncedInput &in) {
  pinMode(in.pin, INPUT_PULLUP);         // internal pull-up as a backup to the external 10k
  in.raw = in.stable = digitalRead(in.pin);
  in.lastEdgeMs = millis();
}

void updateInput(DebouncedInput &in, uint32_t now) {
  int r = digitalRead(in.pin);
  if (r != in.raw) {                     // raw edge (could be bounce)
    in.raw = r;
    in.lastEdgeMs = now;
    in.edges++;
  }
  if (in.raw != in.stable && now - in.lastEdgeMs >= DEBOUNCE_MS) {   // quiet for 50 ms -> accept
    in.stable = in.raw;
    Serial.printf("[%8lu ms] %s %-15s (raw edges: %u)\n", (unsigned long)now, in.name,
                  in.stable == LOW ? in.lowText : in.highText, in.edges);
    in.edges = 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  initInput(reed);
  initInput(tamper);
  Serial.printf("L03 switch inputs (DET). Start: DOOR %s, LID %s\n",
                reed.stable == LOW ? "CLOSED" : "OPEN",
                tamper.stable == LOW ? "CLOSED" : "OPEN");
}

void loop() {
  uint32_t now = millis();
  updateInput(reed, now);
  updateInput(tamper, now);
}
