# Schematics (rev 1, 2026-11-05)

| File | Board | Contents |
|---|---|---|
| `hub_schematic.pdf` / `.png` | Board 1, HUB | 4×4 keypad (GPIO19, 18, 23, 17 / 16, 4, 32, 33), SSD1306 OLED (I2C SDA 21, SCL 22, 0x3C), BC547 siren driver on GPIO25 (1 kΩ base, buzzer to 5 V), LEDs READY 26 / ARMED 27 / ALARM 14 / FAULT 13 (330 Ω each) |
| `detector_schematic.pdf` / `.png` | Board 2, DETECTOR | HC-SR501 PIR (5 V, OUT → GPIO27); reed MC-38 on GPIO26 and tamper microswitch on GPIO25, each with a 10 kΩ pull-up to 3V3 + 100 nF to GND, switch to GND (fail-safe) |

Pin numbers match `firmware/hub/hub.ino` and `firmware/detector_node/detector_node.ino`; update both if a pin changes.

Regenerate the PDFs: `pip install -r tools/requirements.txt`, then `cd hardware/schematics && python ../../tools/gen_schematics.py`.

Design calculations: [`docs/04_hardware`](../../docs/04_hardware).
