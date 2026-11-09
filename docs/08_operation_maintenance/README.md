# Stage 8: Operation and maintenance

## User guide

| Task | Keypad | Phone |
|---|---|---|
| Arm | PIN, then `A`; leave within 30 s | Join **HomeGuard-Hub** (password `homeguard123`), open `http://192.168.4.1`, enter PIN, tap **Arm** |
| Disarm or silence | PIN, then `#` | Enter PIN, tap **Disarm** |
| Clear a typing error | `*` | Edit the PIN box |
| Check status | OLED and LEDs | Web page (refreshes every 2 s) |

Default PIN: `1234`. Arming is refused while the detector lid is open or the detector is offline.

**LEDs:** READY (green) = disarmed and all OK · ARMED = armed (blinks during delays) · ALARM = alarm · FAULT = detector offline or lid open (blinks during PIN lockout).

**Phone tips:** "Connected, no internet" is normal: the hub's network is local only. Turn mobile data off and type the full `http://192.168.4.1`. A laptop works too.

## Maintenance

| Task | How |
|---|---|
| Clear a tamper alarm | Enter PIN + `#`, then close the lid; FAULT clears when TAMPER_CLEAR arrives. Closing the lid alone never silences an alarm (latching by design) |
| Detector offline | Check the power bank and the distance; the hub returns to ONLINE automatically |
| Change PIN, Wi-Fi password or timings | Edit the configuration block at the top of [`hub.ino`](../../firmware/hub/hub.ino) (`PIN_CODE`, `AP_PASS`, `EXIT_DELAY_MS`, …) and re-flash the hub |
| Replace a board | Flash `L01_board_check`, read the new MAC, update `HUB_MAC` / `DET_MAC` and [`board_ids.md`](../02_specification/board_ids.md), re-flash both |
| PIR adjustment | Sensitivity and time pots on the module; jumper on H; allow 30 s warm-up |
| Working inside the detector | Opening the lid triggers the tamper alarm; unplug the buzzer lead on the hub during bench work |
| Forgot PIN / locked out | Wait 60 s; or press EN on the hub (restarts DISARMED) |

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Detector logs `SEND FAILED` every time | Hub not on channel 1, wrong MAC | Check hub prints `channel 1`; try the hub's AP MAC as `HUB_MAC` |
| `Cannot arm: detector lid open` | Lid not pressing the tamper lever | Close the lid fully |
| `Cannot arm: detector offline` | Detector unpowered or out of range | Power it; wait for `Detector ONLINE` |
| Keypad ignored | 60 s lockout after 3 wrong PINs | Wait |
| Phone page does not load | Mobile data on, `http://` missing | Mobile data off, full address |
| PIR alarms during testing | PIR sees the tester | Point it at a wall until needed |
| OLED blank, `OLED NOT FOUND` | I2C wiring | Re-run the L05 I2C scan |
| `'packet.h' not found` | Sketch folder missing its copy | `python tools/sync_packet_h.py` |

## Known limitations

- One detector only (the packet already carries a node ID for more).
- ESP-NOW link not encrypted.
- PIN fixed in firmware, not stored in non-volatile memory.
- Detector on USB power: no battery monitoring, no deep sleep.
- Local alerts only (siren, OLED, web page); no SMS or cloud notification.
- Duplicate filter remembers only the last sequence number (enough for one detector with stop-and-wait delivery).

## Future work

Multiple detectors, ESP-NOW encryption, battery-powered detector with deep sleep and voltage reporting, PIN change from the keypad stored in flash, RFID disarm, GSM/app alerts, PCB and enclosure.
