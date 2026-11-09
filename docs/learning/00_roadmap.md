# Learning roadmap: Wireless Home Security System

Each lesson = concept from basics -> build step -> check with expected output -> common mistakes.
Lessons 01–05 were built on 2026-11-01; lessons 07–10 were merged into one firmware lesson on 2026-11-05. Lessons 11–12 are covered by the integration, validation and O&M documents.

| # | Lesson | Design-flow stage | Day | Status |
|---|---|---|---|---|
| 01 | ESP32 setup: Arduino IDE, board package, flashing, reading the MAC address | SW implementation (toolchain) | 1 | Done |
| 02 | ESP-NOW basics: send and receive a packet between two boards | Architecture check | 1 | Done |
| 03 | Digital inputs: pull-ups, switch bounce, RC filter, debounce (reed + tamper) | HW design + test | 2 | Done |
| 04 | PIR HC-SR501: how it works, settings, interrupt input | HW design + test | 2 | Done |
| 05 | Hub hardware: transistor siren driver, LEDs, keypad matrix, I2C OLED (L05 + L06 merged) | HW design + test | 2 | Done |
| 06 | (merged into lesson 05) | HW + SW | 2 | Merged |
| 07 | Packet design: sequence number, CRC, ACK, retry, heartbeat | SW design | 3 | Done (L07–L10 merged → [07_firmware_protocol_fsm.md](07_firmware_protocol_fsm.md)) |
| 08 | Finite state machine + non-blocking timing with millis() | SW design + implementation | 3 | Merged into 07 |
| 09 | Supervision, tamper and PIN lockout (dependability) | SW implementation + test | 3 | Merged into 07 |
| 10 | Wi-Fi access point + web page on the hub (ESP-NOW on the same channel) | SW implementation | 4 | Merged into 07 |
| 11 | Integration and latency measurement from serial logs | Integration | 4 | Covered in [../07_integration_validation](../07_integration_validation) |
| 12 | Validation, report and demo | Validation + O&M | 5 | Covered in [../07_integration_validation](../07_integration_validation), [../08_operation_maintenance](../08_operation_maintenance), [../report](../report) |
