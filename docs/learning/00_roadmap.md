# Learning roadmap: Wireless Home Security System

Each lesson = concept from basics -> build step -> check with expected output -> common mistakes.
Say **"next"** to start the next lesson. Compressed plan: all lessons run on 2026-11-01 (single-day build).

| # | Lesson | Design-flow stage | Day | Status |
|---|---|---|---|---|
| 01 | ESP32 setup: Arduino IDE, board package, flashing, reading the MAC address | SW implementation (toolchain) | 1 | Done |
| 02 | ESP-NOW basics: send and receive a packet between two boards | Architecture check | 1 | In progress |
| 03 | Digital inputs: pull-ups, switch bounce, RC filter, debounce (reed + tamper) | HW design + test | 2 | Not started |
| 04 | PIR HC-SR501: how it works, settings, interrupt input | HW design + test | 2 | Not started |
| 05 | Transistor as a switch: siren driver and LEDs | HW design + test | 2 | Not started |
| 06 | Keypad matrix scanning and I2C OLED | HW + SW | 2 | Not started |
| 07 | Packet design: sequence number, CRC, ACK, retry, heartbeat | SW design | 3 | Not started |
| 08 | Finite state machine + non-blocking timing with millis() | SW design + implementation | 3 | Not started |
| 09 | Supervision, tamper and PIN lockout (dependability) | SW implementation + test | 3 | Not started |
| 10 | Wi-Fi access point + web page on the hub (ESP-NOW on the same channel) | SW implementation | 4 | Not started |
| 11 | Integration and latency measurement from serial logs | Integration | 4 | Not started |
| 12 | Validation T1-T7, report and demo | Validation + O&M | 5 | Not started |
