// debounced_input.h - debounced switch input (pull-up, switch to GND)
// Kept in a header so the Arduino IDE's automatic prototypes cannot be
// placed before the struct definition.
#pragma once
#include <Arduino.h>

struct DebouncedInput {
  int         pin;
  const char *name;
  const char *lowText;     // meaning of LOW (switch closed)
  const char *highText;    // meaning of HIGH (switch open)
  int         raw;         // last raw reading
  int         stable;      // debounced state
  uint32_t    lastEdgeMs;  // time of last raw change
  uint16_t    edges;       // raw edges since last stable change
};

inline void initInput(DebouncedInput &in) {
  pinMode(in.pin, INPUT_PULLUP);
  in.raw = in.stable = digitalRead(in.pin);
  in.lastEdgeMs = millis();
}

// Returns true when the debounced state changed (and prints it).
inline bool updateInput(DebouncedInput &in, uint32_t now, uint32_t debounceMs) {
  int r = digitalRead(in.pin);
  if (r != in.raw) { in.raw = r; in.lastEdgeMs = now; in.edges++; }
  if (in.raw != in.stable && now - in.lastEdgeMs >= debounceMs) {
    in.stable = in.raw;
    Serial.printf("[%8lu ms] %s %-15s (raw edges: %u)\n", (unsigned long)now, in.name,
                  in.stable == LOW ? in.lowText : in.highText, in.edges);
    in.edges = 0;
    return true;
  }
  return false;
}
