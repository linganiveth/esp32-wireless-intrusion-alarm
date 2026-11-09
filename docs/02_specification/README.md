# Stage 2: Specification

Every requirement turned into numbers and interfaces that can be designed against and tested.

- [`protocol.md`](protocol.md): ESP-NOW packet format and message types
- [`board_ids.md`](board_ids.md): MAC addresses of the two boards used in this project

## Measurable specification

| Parameter | Specification | Traces to |
|---|---|---|
| Radio link | ESP-NOW, 2.4 GHz, Wi-Fi channel 1, peer-to-peer by MAC, no router | NFR4, NFR5 |
| Packet | 12-byte packed struct: node ID, type, seq, battery, uptime, CRC-16/CCITT | NFR2 |
| Reliability | ACK within 100 ms; up to 3 transmissions; duplicates filtered by seq | NFR2 |
| Supervision | Heartbeat every 5 s; OFFLINE after 15 s without any packet | FR7, NFR3 |
| Switch inputs | Switch to GND + 10 kΩ pull-up + 100 nF (τ = 1 ms); 50 ms software debounce | FR2, FR3, NFR3 |
| PIR input | 3.3 V digital output on an interrupt pin; 30 s warm-up ignored | FR1 |
| Alarm timing | Exit 30 s; entry 15 s; siren max 3 min per alarm | FR5, FR6 |
| Security | 4-digit PIN; 3 wrong attempts → 60 s lockout (keypad and phone) | FR4, FR8 |
| User interface | 4×4 keypad, 128×64 OLED (I2C 0x3C), 4 LEDs, buzzer, web page at http://192.168.4.1 | FR4, FR6 |
| Response time | Sensor event → siren on < 1 s | NFR1 |

## Timing constants (firmware)

| Constant | Value | Node | File |
|---|---|---|---|
| `DEBOUNCE_MS` | 50 ms | Detector | `detector_node.ino` |
| `PIR_WARMUP_MS` | 30 000 ms | Detector | `detector_node.ino` |
| `HEARTBEAT_MS` | 5 000 ms | Detector | `detector_node.ino` |
| `ACK_TIMEOUT_MS` | 100 ms | Detector | `detector_node.ino` |
| `MAX_RETRIES` | 3 | Detector | `detector_node.ino` |
| `EXIT_DELAY_MS` | 30 000 ms | Hub | `hub.ino` |
| `ENTRY_DELAY_MS` | 15 000 ms | Hub | `hub.ino` |
| `OFFLINE_MS` | 15 000 ms | Hub | `hub.ino` |
| `LOCKOUT_MS` | 60 000 ms | Hub | `hub.ino` |
| `SIREN_MAX_MS` | 180 000 ms | Hub | `hub.ino` |

## Pin map

**Hub (Board 1)**

| Signal | ESP32 pin | Notes |
|---|---|---|
| Siren driver | GPIO25 | 1 kΩ to BC547 base; buzzer from 5 V |
| LED READY / ARMED / ALARM / FAULT | GPIO26 / 27 / 14 / 13 | 330 Ω each |
| OLED SDA / SCL | GPIO21 / GPIO22 | SSD1306, address 0x3C, 3.3 V |
| Keypad rows R1–R4 (pins 1–4) | GPIO19, 18, 23, 17 | |
| Keypad columns C1–C4 (pins 5–8) | GPIO16, 4, 32, 33 | |

**Detector (Board 2)**

| Signal | ESP32 pin | Notes |
|---|---|---|
| PIR OUT | GPIO27 | Interrupt on rising edge; PIR VCC from VIN (5 V) |
| Door reed switch | GPIO26 | 10 kΩ pull-up + 100 nF; LOW = door closed |
| Tamper switch (COM–NO) | GPIO25 | 10 kΩ pull-up + 100 nF; LOW = lid closed |
| TX indicator | GPIO2 | On-board LED, 30 ms flash per packet |

## Wi-Fi access point (hub)

SSID `HomeGuard-Hub`, password `homeguard123`, channel 1, address `192.168.4.1`. Default PIN `1234`.
