# Lesson 04: PIR motion sensor HC-SR501 and interrupt inputs

Design-flow stage: Hardware design + test (HT2) · Detector node (DET, Board 2)

## 1. Concept

**What a PIR sees.** Every warm body emits infrared. The sensor element is *pyroelectric*: it produces a tiny voltage when the infrared falling on it **changes**. It has two halves wired against each other, so a still, warm room cancels out. When a person walks across the field of view, one half sees them before the other, giving a +/- swing. The white dome is a **Fresnel lens** that splits the room into many zones so movement keeps crossing zone edges. PIR = **Passive** InfraRed: it emits nothing, so it draws only ~65 µA.

**What the module does.** A controller chip (BISS0001) amplifies that swing and turns it into a clean digital output:

| Control | Setting for this project | Effect |
|---|---|---|
| Sensitivity pot | middle | detection distance, roughly 3-7 m |
| Time-delay pot | minimum (shortest) | how long OUT stays HIGH after motion, ~3 s at minimum |
| Trigger jumper | H (repeat) | OUT stays HIGH while motion continues |
| Warm-up | 30-60 s after power-on | output is unreliable until it settles |

Pot direction varies between modules; verify by measuring the HIGH time.

**Power and levels.** The module needs ~5 V on VCC (use the DevKit's VIN/5V pin, which is USB 5 V) but its OUT pin swings 0 / 3.3 V from an on-board regulator, so it connects straight to an ESP32 GPIO. Verify OUT HIGH ≤ 3.3 V with the multimeter before connecting.

**Interrupt vs polling.** Polling checks the pin every loop; an **interrupt** makes the hardware call a function the instant the pin changes. Rules for the interrupt function (ISR): mark it `IRAM_ATTR`, keep it a few lines, set a flag and a timestamp, no printing. loop() handles the rest. The timestamp is what we later use to measure detection-to-siren latency (TR1).

## 2. Build step

| HC-SR501 pin | Connect to |
|---|---|
| VCC | ESP32 **VIN** (5 V) |
| OUT | **GPIO27** |
| GND | GND (− rail) |

Pins are usually labelled under the dome (lift it gently). Keep the L03 reed and tamper wiring in place. Flash `firmware/test_sketches/L04_detector_inputs/` to DET.

## 3. Check

```
L04 detector inputs. Door CLOSED, lid CLOSED. PIR warming up 30 s...
[   30001 ms] PIR ready (output now LOW)
[   41230 ms] MOTION START
[   44380 ms] MOTION END   (output HIGH for 3150 ms)
[   52004 ms] DOOR OPEN            (raw edges: 1)
```
Pass (HT2):
- Sitting still -> no MOTION lines for 60 s.
- Walking across the sensor at **1 m, 2 m and 4 m** -> MOTION START each time.
- Multimeter on OUT: ≈ 3.3 V during motion, ≈ 0 V idle.
- Door and lid still behave as in L03.

| Distance | Detected? | HIGH time (ms) |
|---|---|---|
| 1 m | | |
| 2 m | | |
| 4 m | | |

## 4. Common mistakes

| Symptom | Cause | Fix |
|---|---|---|
| Random MOTION lines with nobody there | Warm-up not finished, sunlight, fan, AC vent | Wait 60 s; point away from windows and vents |
| OUT stuck HIGH | Time pot at maximum (minutes) or jumper on L with constant motion | Turn time pot to minimum; jumper to H |
| Never triggers | VCC on 3.3 V instead of 5 V | Use VIN |
| Detects only very close | Sensitivity at minimum | Turn sensitivity pot towards maximum |
| Walking straight at it detects late | PIR senses movement *across* zones best | Walk across its view for tests |
| `'DebouncedInput' was not declared in this scope` (seen 2026-11-01) | Arduino IDE inserts automatic function prototypes above the first function (the ISR), before the struct is defined | Struct + helpers moved to `debounced_input.h` in the sketch folder; headers are not rewritten by the IDE |
