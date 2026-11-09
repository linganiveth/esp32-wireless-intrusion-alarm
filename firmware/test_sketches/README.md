# Test sketches (hardware bring-up)

Small sketches that test one block at a time before the final firmware. Each matches a lesson in [`docs/learning`](../../docs/learning) and a hardware test in [`docs/06_testing`](../../docs/06_testing).

| Sketch | Board | Tests | Expected result |
|---|---|---|---|
| `L01_board_check` | each | Toolchain, chip info, **STA MAC address** | Report every 5 s, on-board LED blinks; note the MAC |
| `L02_espnow_hub` + `L02_espnow_det` | hub + detector | ESP-NOW link, round-trip time, loss (HT6) | ≥ 99/100 pings answered, RTT a few ms |
| `L03_switch_inputs` | detector | Reed + tamper inputs, debounce, raw bounce count (HT3) | One line per open/close |
| `L04_detector_inputs` | detector | PIR interrupt + switch inputs (HT2, HT3) | MOTION START/END with pulse length |
| `L05_hub_io` | hub | I2C scan + OLED, LEDs, keypad, siren (HT4, HT5) | OLED at 0x3C, 16 keys, A/B/C/D control LEDs and siren |

The L02 sketches contain the MAC addresses of this project's boards; replace them with your own from L01.
