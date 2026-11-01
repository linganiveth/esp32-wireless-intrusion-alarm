# Firmware

| Folder | Board | Purpose |
|---|---|---|
| `common/` | both | Shared packet definition (`packet.h`), copied into each sketch |
| `detector_node/` | ESP32 #1 | PIR, reed, tamper -> events + heartbeat over ESP-NOW |
| `hub/` | ESP32 #2 | Alarm state machine, keypad, OLED, siren, LEDs, web page |
| `test_sketches/` | either | Small sketches used in hardware tests (HT) and lessons |

Arduino rule: a sketch folder and its `.ino` file must have the same name.
