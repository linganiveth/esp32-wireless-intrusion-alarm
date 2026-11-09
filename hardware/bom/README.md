# Bill of materials

Full list with specifications, purposes, priorities and a budget summary: [`Home_Security_Component_List.xlsx`](Home_Security_Component_List.xlsx). Prices are approximate Indian retail (October 2026).

## Core parts (≈ ₹1,970)

| # | Component | Specification | Qty | Used in |
|---|---|---|---|---|
| 1 | ESP32 DevKit V1 (ESP32-WROOM-32) | CP2102 or CH340 USB | 2 | Hub + detector |
| 2 | Micro-USB **data** cable | Not charge-only | 2 | Flashing, power, serial logs |
| 3 | PIR HC-SR501 | 5 V supply, 3.3 V output, two pots + H/L jumper | 1 | Detector GPIO27 |
| 4 | MC-38 wired reed door sensor | Switch + magnet | 1 | Detector GPIO26 |
| 5 | Lever microswitch (SS-5GL / KW11) | Normally open | 2 | Tamper, detector GPIO25 |
| 6 | 4×4 membrane keypad | 8-pin | 1 | Hub |
| 7 | SSD1306 0.96" OLED | I2C, 4-pin, address 0x3C | 1 | Hub GPIO21/22 |
| 8 | Active buzzer | 5 V, active type | 1 | Hub siren |
| 9 | BC547 NPN transistor | — | 3 | Siren driver (+ spares) |
| 10 | 1N4148 diode | — | 2 | Flyback, only for a magnetic buzzer |
| 11 | 5 mm LEDs | green, yellow, red, blue | 6 | Hub status |
| 12 | Resistors | 10 kΩ ×10, 1 kΩ ×5, 330 Ω ×6 | — | Pull-ups, base, LEDs |
| 13 | Ceramic capacitor 100 nF (104) | — | 10 | RC filter, decoupling |
| 14 | ABS project box | ~100 × 60 × 25 mm | 1 | Detector enclosure |
| 15 | Breadboards | 830 + 400 points | 2 | Hub, detector |
| 16 | Jumper wires, header strip | M–M, M–F, 40-pin | — | Wiring |

**Also used:** USB charger (hub), USB power bank (detector), digital multimeter (HT1–HT4).

**Optional (not used in the core build):** spare PIR, RC522 RFID kit, 18650 cell + TP4056 for a battery detector, third ESP32 for a second detector.
