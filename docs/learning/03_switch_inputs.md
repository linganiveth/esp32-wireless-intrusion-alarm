# Lesson 03: Switch inputs - pull-ups, bounce, RC filter, debounce

Design-flow stage: Hardware design + hardware test (HT3) · Detector node (DET, Board 2)

## 1. Concept

**A switch is not a voltage.** A closed switch connects two points; an open switch connects nothing. A GPIO input needs a defined voltage in both cases, so we add a **pull-up resistor**: switch open -> the resistor pulls the pin to 3.3 V (HIGH); switch closed -> the switch shorts the pin to GND (LOW). Without it, an open switch leaves the pin "floating" and it reads random values.

| Switch | Door / lid | Pin |
|---|---|---|
| Reed closed (magnet near) | Door CLOSED | LOW (0 V) |
| Reed open (magnet away) | Door OPEN | HIGH (3.3 V) |
| Tamper pressed by lid | Lid CLOSED | LOW |
| Tamper released | Lid OPEN = tamper | HIGH |

Wiring "switch to GND" also makes a **cut wire read as OPEN**, the alarm side, which is the fail-safe choice.

**Bounce.** Metal contacts bounce for 1-10 ms when they close, producing several HIGH/LOW edges. The firmware would see "open, closed, open, closed". Two layers fix it:

1. **RC filter (hardware):** 10 kΩ pull-up + 100 nF to GND. Time constant τ = R × C = 10 kΩ × 100 nF = **1 ms**. It smooths fast spikes and noise picked up by a long wire.
2. **Software debounce:** accept a new state only after the raw reading has been steady for **50 ms** (DR4 in the requirements).

**Why 10 kΩ?** Current when closed = 3.3 V / 10 kΩ = 0.33 mA (tiny), yet strong enough to resist noise. The ESP32's internal pull-up (~45 kΩ) is also enabled as a backup.

## 2. Build step

On the DET breadboard (verify the MC-38 with a multimeter first: continuity when the magnet is close):

| Part | Connection |
|---|---|
| Reed switch MC-38 | wire 1 -> GPIO26, wire 2 -> GND; magnet on the moving door |
| 10 kΩ | GPIO26 -> 3V3 |
| 100 nF | GPIO26 -> GND (close to the ESP32) |
| Tamper microswitch | COM -> GND, NO -> GPIO25 (lever pressed by the box lid) |
| 10 kΩ | GPIO25 -> 3V3 |
| 100 nF | GPIO25 -> GND |

Flash `firmware/test_sketches/L03_switch_inputs/` to DET, Serial Monitor 115200.

## 3. Check

Expected:
```
L03 switch inputs (DET). Start: DOOR CLOSED, LID CLOSED
[    5123 ms] DOOR OPEN            (raw edges: 1)
[    7012 ms] DOOR CLOSED          (raw edges: 1)
[    9500 ms] LID  OPEN -> TAMPER  (raw edges: 1)
[   11020 ms] LID  CLOSED          (raw edges: 2)
```
Pass:
- Each physical open/close prints exactly one line (no repeated lines for one action).
- HT3 with a multimeter on the GPIO pin: door shut ≈ 0 V, open ≈ 3.3 V; lid on ≈ 0 V, off ≈ 3.3 V.

**Experiment (2 min, good for the viva):** remove the 100 nF capacitor on the tamper input and press the switch quickly 5 times. "raw edges" rises (often 2-10) but the debounced output still prints one line per press. That shows why both layers exist. Put the capacitor back.

Record for HT3:

| Input | Closed (V) | Open (V) | Raw edges with C | Raw edges without C |
|---|---|---|---|---|
| Reed GPIO26 | | | | |
| Tamper GPIO25 | | | | |

## 4. Common mistakes

| Symptom | Cause | Fix |
|---|---|---|
| Door always OPEN | Reed is normally-open type and magnet too far, or wire on wrong pin | Check continuity with magnet within ~10 mm; check GPIO26 |
| Readings flicker without touching | No pull-up / loose jumper | Check the 10 kΩ to 3V3 |
| Lid logic inverted | Wired to NC instead of NO | Move the wire to NO |
| Many lines per press | Debounce not running | Make sure loop() has no delay() |
