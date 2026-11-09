/*
 * L04_detector_inputs.ino  -  Lesson 04: PIR + door + tamper on the detector (DET, Board 2)
 * PIR HC-SR501 on GPIO27 via interrupt; reed (GPIO26) and tamper (GPIO25) debounced as in L03.
 * Prints MOTION START / END with pulse length, and door / lid changes. Serial Monitor 115200.
 */
#include "debounced_input.h"

const int PIN_PIR    = 27;   // HC-SR501 OUT (3.3 V logic), HIGH = motion
const int PIN_REED   = 26;   // LOW = door CLOSED
const int PIN_TAMPER = 25;   // LOW = lid CLOSED
const uint32_t DEBOUNCE_MS   = 50;
const uint32_t PIR_WARMUP_MS = 30000;   // HC-SR501 needs ~30-60 s after power-up

// ---------- PIR: interrupt sets a flag, loop() does the work ----------
volatile bool     pirEdge = false;
volatile uint32_t pirEdgeMs = 0;

void IRAM_ATTR onPirChange() {          // runs on BOTH edges; keep it tiny
  pirEdge   = true;
  pirEdgeMs = millis();
}

// ---------- Debounced switches (see debounced_input.h) ----------
DebouncedInput reed   = {PIN_REED,   "DOOR", "CLOSED", "OPEN",           HIGH, HIGH, 0, 0};
DebouncedInput tamper = {PIN_TAMPER, "LID ", "CLOSED", "OPEN -> TAMPER", HIGH, HIGH, 0, 0};

bool pirReady = false;
bool pirHigh  = false;
uint32_t motionStartMs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIN_PIR, INPUT);               // PIR drives the line itself: no pull-up
  attachInterrupt(digitalPinToInterrupt(PIN_PIR), onPirChange, CHANGE);
  initInput(reed);
  initInput(tamper);
  Serial.printf("L04 detector inputs. Door %s, lid %s. PIR warming up %lu s...\n",
                reed.stable == LOW ? "CLOSED" : "OPEN", tamper.stable == LOW ? "CLOSED" : "OPEN",
                (unsigned long)(PIR_WARMUP_MS / 1000));
}

void loop() {
  uint32_t now = millis();

  if (!pirReady && now >= PIR_WARMUP_MS) {
    pirReady = true;
    pirHigh = digitalRead(PIN_PIR);
    Serial.printf("[%8lu ms] PIR ready (output now %s)\n", (unsigned long)now, pirHigh ? "HIGH" : "LOW");
  }

  if (pirEdge) {
    noInterrupts();
    pirEdge = false;
    uint32_t t = pirEdgeMs;
    interrupts();
    bool level = digitalRead(PIN_PIR);
    if (pirReady && level != pirHigh) {
      pirHigh = level;
      if (level) {
        motionStartMs = t;
        Serial.printf("[%8lu ms] MOTION START\n", (unsigned long)t);
      } else {
        Serial.printf("[%8lu ms] MOTION END   (output HIGH for %lu ms)\n",
                      (unsigned long)t, (unsigned long)(t - motionStartMs));
      }
    }
  }

  updateInput(reed, now, DEBOUNCE_MS);
  updateInput(tamper, now, DEBOUNCE_MS);
}
