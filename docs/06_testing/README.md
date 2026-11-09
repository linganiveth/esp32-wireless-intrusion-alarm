# Stages 4 and 5: Hardware and software testing

| File | Contents |
|---|---|
| [`hardware_tests.md`](hardware_tests.md) | HT1–HT6: power, PIR, switch inputs, siren driver, LEDs/keypad/OLED, radio link |
| [`software_tests.md`](software_tests.md) | ST1–ST11: event delivery, arming, delays, tamper, supervision, lockout, phone page, retries |
| [`logs/`](logs) | Serial logs used as evidence (name: `L<lesson>_<topic>_<date>.txt`) |

## How the tests are run

- **Hardware tests** use the unit-test sketches in [`firmware/test_sketches`](../../firmware/test_sketches) (L01–L05) and a multimeter. Procedures are in the matching lesson in [`../learning`](../learning).
- **Software tests** use the final firmware with both serial monitors open at 115200 baud. Procedures: [`../learning/07_firmware_protocol_fsm.md`](../learning/07_firmware_protocol_fsm.md), section 3.2.
- Suggested order for ST tests (about 15 min): ST1 → ST2 → ST3 → ST4 → ST5 → ST6 → ST9 (lid still open) → ST8 → ST7 → ST10 → ST11.
- Save each serial monitor output to `logs/` and fill in the Result column.
