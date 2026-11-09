# Lesson 05: Hub hardware - transistor siren driver, LEDs, keypad matrix, I2C OLED

Design-flow stage: Hardware design + test (HT4, HT5) · Hub (HUB, Board 1). Lessons 05 and 06 merged on 2026-11-01 to fit the single-day build.

## 1. Concept

**Transistor as a switch (siren).** An ESP32 pin should source only a few mA, but the buzzer takes ~30 mA at 5 V. A BC547 NPN transistor switches the buzzer's ground side: a small base current turns on a larger collector current.

| Quantity | Value |
|---|---|
| Base current Ib | (3.3 V − 0.7 V) / 1 kΩ = 2.6 mA |
| Max collector current it can drive | hFE (≥ 110) × 2.6 mA ≈ 286 mA |
| Buzzer current | ~30 mA, far below 286 mA, so the transistor is fully ON (saturated) |
| Collector-emitter voltage when ON | ≤ ~0.2 V (this is what HT4 measures) |

Low-side switching (transistor between buzzer and GND) works because the emitter sits at 0 V, so 3.3 V on the base is enough to turn it on even though the buzzer runs from 5 V.

**LED resistor.** (3.3 V − ~2.0 V LED drop) / 330 Ω ≈ 4 mA: bright enough, safe for the pin.

**Keypad matrix.** 16 keys use only 8 wires: 4 rows × 4 columns. The library drives one row LOW at a time and reads which column goes LOW (columns have pull-ups). Row r + column c = the key at that crossing. That is 8 GPIOs instead of 16.

**I2C OLED.** Two wires (SDA data, SCL clock) shared by many devices, each with a 7-bit address. The SSD1306 is normally at 0x3C. The I2C scan proves the wiring before any display code runs.

## 2. Build step

| Part | Connections (verify pinouts against the part's datasheet/silkscreen) |
|---|---|
| Buzzer (active, 5 V) | + -> VIN (5 V); − -> BC547 collector |
| BC547 | Collector -> buzzer −; Emitter -> GND; Base -> 1 kΩ -> **GPIO25**. Common BC547 pinout, flat face towards you, legs down: C B E (verify) |
| Diode (only if magnetic buzzer) | across the buzzer, stripe (cathode) to +5 V |
| LEDs | GPIO26 READY, GPIO27 ARMED, GPIO14 ALARM, GPIO13 FAULT -> 330 Ω -> LED long leg (anode); short leg -> GND |
| OLED | VCC -> 3V3, GND -> GND, SDA -> **GPIO21**, SCL -> **GPIO22** |
| Keypad (8-pin, keys facing you, pins left to right) | 1-4 rows -> GPIO19, 18, 23, 17; 5-8 columns -> GPIO16, 4, 32, 33 |

Libraries (Sketch -> Include Library -> Manage Libraries): **Adafruit SSD1306** (accept Adafruit GFX + BusIO), **Keypad** by Mark Stanley. Flash `firmware/test_sketches/L05_hub_io/` to HUB.

## 3. Check

```
I2C scan:
  device at 0x3C
OLED OK at 0x3C
LED walk:
  READY (GPIO26) ON
  ARMED (GPIO27) ON
  ALARM (GPIO14) ON
  FAULT (GPIO13) ON
Siren chirp 100 ms
Ready: press keys (A armed LED, B beep, C all LEDs, D siren on/off)
Key '1'  (count 1)
```
Pass:
- HT5: I2C finds 0x3C, OLED shows text, each LED lights in turn, all 16 keys print the right character.
- HT4: B beeps; D turns the siren on -> multimeter between BC547 collector and emitter reads ≤ 0.2 V; D again -> off.

| Test | Expected | Measured |
|---|---|---|
| I2C address | 0x3C | |
| 16 keys correct | 16/16 | |
| 4 LEDs | 4/4 | |
| Siren beeps on B | yes | |
| V(C-E) with siren ON | ≤ 0.2 V | |

## 4. Common mistakes

| Symptom | Cause | Fix |
|---|---|---|
| I2C "none found" | SDA/SCL swapped, OLED on 5 V-only module, loose GND | Swap 21/22; check VCC and GND |
| Wrong key characters (e.g. 1 prints as A) | Keypad connector reversed or rows/cols swapped | Reverse the 8 wires, or swap rowPins/colPins arrays |
| Some keys dead | One row or column wire loose | The dead keys share a row or column: reseat that wire |
| Siren always on | Collector/emitter swapped or base tied high | Recheck pinout; GPIO25 must be LOW at boot |
| Siren weak / silent | Passive buzzer (needs a tone) or buzzer on 3.3 V | Use an active 5 V buzzer on VIN |
| LED never lights | LED reversed | Long leg to the resistor side |

## 5. Step-by-step wiring (breadboard, 830 points)

Unplug USB while wiring. Row numbers are suggestions; any free row works if you stay consistent.

**Step 1 - Place the ESP32.** Straddle the centre gap, USB end at the board's edge (rows 1-15). Read the pin names printed next to each pin; pin order differs between DevKit makers. If the board is so wide that no hole is free on one side, leave it off the breadboard and use male-female jumpers from its pins.

**Step 2 - Two supplies on the rails.**
| Rail | Connect | Feeds |
|---|---|---|
| Top red (+) | ESP32 **3V3** | OLED |
| Bottom red (+) | ESP32 **VIN** (5 V from USB) | Buzzer |
| Both blue (−) | ESP32 **GND**, plus one jumper joining top − to bottom − | Everything |
Mark the top + rail "3V3" and the bottom + rail "5V" with tape. Never connect a 5 V rail to a GPIO.

**Step 3 - OLED (4 jumpers).** VCC -> top + (3V3); GND -> −; SDA -> GPIO21; SCL -> GPIO22.

**Step 4 - Four LEDs (repeat four times, rows 30-45).**
1. Jumper: GPIO26 -> row 30.
2. 330 Ω: row 30 -> row 32.
3. LED: long leg (anode) in row 32, short leg in the − rail.
Repeat: GPIO27 -> rows 34/36 (ARMED), GPIO14 -> rows 38/40 (ALARM), GPIO13 -> rows 42/44 (FAULT).

**Step 5 - Siren driver (rows 48-55).**
1. BC547 in rows 50, 51, 52, flat face towards you: C = row 50, B = row 51, E = row 52 (verify with the datasheet; C-B-E is the common pinout).
2. Emitter row 52 -> − rail.
3. 1 kΩ: row 51 (base) -> row 48. Jumper: GPIO25 -> row 48.
4. Buzzer − (short leg / unmarked) in row 50 (collector); buzzer + (long leg / "+" mark) -> bottom + rail (5 V).
5. Magnetic buzzer only: diode across the buzzer, stripe to the + leg.

**Step 6 - Keypad (8 male-male jumpers into its connector).** Keys facing you, connector pins numbered 1-8 from left:
| Keypad pin | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| Function | Row 1 | Row 2 | Row 3 | Row 4 | Col 1 | Col 2 | Col 3 | Col 4 |
| ESP32 | GPIO19 | GPIO18 | GPIO23 | GPIO17 | GPIO16 | GPIO4 | GPIO32 | GPIO33 |

**Step 7 - Check before power (multimeter, continuity/beep mode).**
- 3V3 rail to − rail: no beep.
- 5V rail to − rail: no beep.
- GPIO25 to − rail: no beep (base resistor present).

**Step 8 - Power and test.** Plug USB, upload L05_hub_io, open Serial Monitor at 115200, run the checks in section 3.
