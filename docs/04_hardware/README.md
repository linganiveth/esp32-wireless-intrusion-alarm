# Stage 4: Hardware design

Schematics: [`hardware/schematics`](../../hardware/schematics) (PDF and PNG). Bill of materials: [`hardware/bom`](../../hardware/bom). Test results: [`../06_testing/hardware_tests.md`](../06_testing/hardware_tests.md).

## Detector node

**Fail-safe switch inputs.** Reed and tamper switches connect the pin to GND when closed; a 10 kΩ resistor pulls the pin to 3.3 V when open. Secure state (door shut, lid on) = LOW. Door open, lid removed **or a cut wire** = HIGH, so a broken connection produces an event instead of silence. The internal pull-up is enabled too.

**Why 10 kΩ.** The switches are closed most of the time, so the resistor draws I = 3.3 V / R continuously: 3.3 mA with 1 kΩ, 0.33 mA with 10 kΩ. 100 kΩ would make long wires prone to noise.

**Debounce.** 100 nF from pin to GND gives τ = 10 kΩ × 100 nF = **1 ms** (removes spikes and wire noise). Firmware then requires **50 ms** of stable level before accepting a change. Measured: the lever switch bounced up to 31 times per press; one event was still reported.

**PIR (HC-SR501).** Powered from VIN (5 V, module accepts 4.5–20 V); its on-board regulator gives a 3.3 V digital output wired straight to GPIO27. Jumper on **H** (repeat trigger). Clean digital output, so no debounce; firmware ignores it for 30 s after power-up.

## Hub

**Siren driver.** GPIO25 → 1 kΩ → BC547 base; buzzer between 5 V and collector; emitter to GND.

- I_B = (3.3 − 0.7) V / 1 kΩ = **2.6 mA**
- With h_FE ≥ 100 the transistor could carry ≈ 260 mA, far above the buzzer current → saturated, V_CE(sat) ≤ 0.2 V
- The buzzer gets almost the full 5 V; the GPIO only supplies the base current
- Active buzzer, so no tone generation; add a 1N4148 flyback diode if a magnetic buzzer is used

**LEDs.** GPIO → 330 Ω → LED → GND: I = (3.3 − 2.0) V / 330 Ω ≈ **4 mA** each.

**Keypad.** 4×4 matrix: rows driven one at a time, columns read → 16 keys from 8 GPIOs.

**OLED.** SSD1306 on I2C (SDA 21, SCL 22), 7-bit address 0x3C, 3.3 V; pull-ups are on the module.

## Power budget (estimates from typical datasheet values)

| Load | Detector | Hub |
|---|---|---|
| ESP32 with radio receiving | ≈ 100 mA | ≈ 100 mA |
| PIR idle | ≈ 0.065 mA | — |
| Pull-ups (switches closed) | 0.66 mA | — |
| OLED | — | ≈ 10–20 mA |
| LEDs | — | 4–8 mA |
| Buzzer (alarm only) | — | ≈ 30 mA |
| **Total** | **≈ 100 mA** | **≈ 115–160 mA** |

Both nodes run from 5 V USB. The detector radio is always on, so a 10 000 mAh power bank lasts roughly three to four days; deep sleep is future work.

## Build notes

- Hub on an 830-point breadboard; detector on a half breadboard inside a small ABS box with the PIR looking out through the lid and the tamper lever pressed by the closed lid.
- Check continuity with a multimeter before powering each new block.
- Step-by-step wiring instructions: [`../learning/03_switch_inputs.md`](../learning/03_switch_inputs.md), [`04_pir_sensor.md`](../learning/04_pir_sensor.md), [`05_hub_hardware.md`](../learning/05_hub_hardware.md).
