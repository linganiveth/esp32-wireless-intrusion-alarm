# Firmware

| Folder | Board | Purpose |
|---|---|---|
| [`common/`](common) | both | Master copy of the shared packet definition `packet.h` |
| [`detector_node/`](detector_node) | Board 2 (detector) | PIR, reed and tamper inputs → events + heartbeat over ESP-NOW with ACK/retry |
| [`hub/`](hub) | Board 1 (hub) | Alarm state machine, supervision, keypad, OLED, siren, LEDs, Wi-Fi AP + web page |
| [`test_sketches/`](test_sketches) | either | Unit-test sketches L01–L05 used during hardware bring-up |

## Build environment

- Arduino IDE 2.x, board package **esp32 by Espressif Systems** (core 3.x; 2.x also compiles), board **ESP32 Dev Module**, upload speed 921600 (lower it if uploads fail), Serial Monitor 115200.
- Libraries (Library Manager): **Adafruit SSD1306** (+ Adafruit GFX), **Keypad** by Mark Stanley, Alexander Brevig. `WiFi`, `WebServer`, `esp_now`, `esp_wifi` come with the core.
- Open the `.ino` file; a sketch folder and its `.ino` must have the same name, and every header it includes must be inside the folder.

## Configure before flashing

| File | Setting | Default |
|---|---|---|
| `detector_node/detector_node.ino` | `HUB_MAC` (hub's STA MAC from L01) | `FC:E8:C0:75:C1:94` |
| `hub/hub.ino` | `DET_MAC` (detector's STA MAC from L01) | `D0:EF:76:33:EE:44` |
| `hub/hub.ino` | `PIN_CODE`, `AP_SSID`, `AP_PASS` | `1234`, `HomeGuard-Hub`, `homeguard123` |
| both | `WIFI_CHANNEL` | `1` (must be equal on both) |
| both | timing constants | see [`docs/02_specification`](../docs/02_specification) |

After editing `common/packet.h`, run `python tools/sync_packet_h.py` to refresh the copies in `detector_node/` and `hub/`.

## Flash and check

1. Flash `detector_node` to the detector and `hub` to the hub.
2. Hub serial output:
   ```
   HUB ready. Wi-Fi 'HomeGuard-Hub' (password homeguard123), open http://192.168.4.1
   STA MAC FC:E8:C0:75:C1:94  AP MAC FC:E8:C0:75:C1:95  channel 1  OLED OK
   [ 5xx ms] Hub started, DISARMED
   [ xxxx ms] Detector ONLINE
   ```
3. Detector serial output:
   ```
   DETECTOR ready on channel 1. Door CLOSED, lid CLOSED. PIR warming up 30 s
   [ xxxx ms] TX   DOOR_CLOSED seq=1
   [ xxxx ms] ACK  DOOR_CLOSED seq=1  event->ACK 3 ms (tries 1)
   ```
4. Run the tests in [`docs/06_testing`](../docs/06_testing).

If the detector prints `SEND FAILED` for every event, set `HUB_MAC` to the hub's **AP MAC** printed above and re-flash the detector.
