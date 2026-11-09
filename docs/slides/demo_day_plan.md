# Demo day plan — 2026-11-06

## A. Tonight before leaving (5 min)
- [ ] Both boards flashed with the FINAL firmware (detector_node on Board 2, hub on Board 1)
- [ ] Commit and push everything (`git add . && git commit -m "demo-ready firmware" && git push`)
- [ ] Charge: phone, laptop, and a power bank for the detector (so it is visibly wireless)
- [ ] Pack: 2 USB cables (+1 spare), power bank, multimeter, small screwdriver (PIR pots), magnet + reed, tape

## B. Morning bench check (45 min, in this order; stop and fix if any fail)
| # | Check | Pass | Time |
|---|---|---|---|
| 1 | Power hub (USB to laptop, Serial Monitor open) | `HUB ready`, `OLED OK`, OLED shows DISARMED | 2 min |
| 2 | Power detector from the power bank | Hub log: `Detector ONLINE`, READY LED on. Wait 30 s for `PIR ready` | 2 min |
| 3 | Keypad: `1234A` then `1234#` | EXIT DELAY then DISARMED | 2 min |
| 4 | Door open/close 5x (disarmed) | 10 events on hub, OLED "Last:" updates | 2 min |
| 5 | Arm → wait 30 s → open door → `1234#` within 15 s | ENTRY DELAY → DISARMED, no siren | 2 min |
| 6 | Arm → wait 30 s → wave at PIR | Instant ALARM, siren pattern | 2 min |
| 7 | Disarm → open detector lid | ALARM even though disarmed; `1234#` silences | 2 min |
| 8 | Unplug the detector power bank | ~15 s → DETECTOR OFFLINE, FAULT LED, chirp; plug back → ONLINE | 1 min |
| 9 | `0000A` x3 | KEYPAD LOCKED 60 s | 2 min |
| 10 | Phone: join HomeGuard-Hub, mobile data OFF, `http://192.168.4.1` | Page loads, Arm/Disarm from phone works | 5 min |
| 11 | Copy both serial logs to `docs/06_testing/logs/` | Saved | 3 min |
| 12 | Record a 2-3 min video of runs 5-8 on the phone | Backup if the live demo fails | 5 min |

Optional, only if time: multimeter (door/tamper pin 0 V/3.3 V, BC547 collector ≤ 0.2 V while siren on), range at 10 m.

## C. Demo script (6-8 min)
1. **Problem + design flow (1 min):** wireless intrusion detection like Ajax; requirements → spec → architecture → HW/SW → integration → validation.
2. **Architecture (1 min):** detector ESP32 (PIR, door reed, tamper) → ESP-NOW, 12-byte packet with seq + CRC + ACK/retry → hub ESP32 (state machine, keypad, OLED, siren, LEDs, phone page).
3. **Live (5 min):**
   - Show the phone page and the OLED: DISARMED, detector OK.
   - Arm with the keypad. **Use the 30 s exit delay to explain the state machine** (show the diagram).
   - Open the door → entry delay → disarm with PIN → "authorised user".
   - Arm from the phone → wave at the PIR → instant alarm → disarm from the phone.
   - Open the detector lid while disarmed → alarm → "tamper is 24-hour protection".
   - Unplug the detector → offline fault after 15 s → "a smashed sensor cannot fail silently".
   - Point at the detector serial log: `event->ACK x ms` → measured latency and zero lost events.
4. **Close (30 s):** limitations + future work.

Keep the PIR facing a wall until step "wave at the PIR", otherwise people walking past trigger it.

## D. Likely questions — short answers
| Question | Answer |
|---|---|
| Why two microcontrollers? | Sensor and control panel are physically separate in a real house; the radio link, its reliability and its supervision are the core engineering problem. |
| Why ESP-NOW, not Wi-Fi/MQTT? | No router needed, peer-to-peer, ~3 ms latency, works if home Wi-Fi is down. |
| What if a packet is lost? | Sequence number + CRC + ACK; detector retries up to 3 times; hub ignores duplicates by seq. |
| What if someone breaks the detector? | Tamper switch → alarm in any state; if it goes silent, heartbeat supervision flags OFFLINE in 15 s. |
| Why does closing the lid not stop the alarm? | Alarms latch; only a valid PIN clears them, so an intruder cannot silence it. |
| Fail-safe wiring? | Switches pull to GND with pull-ups; a cut wire reads as OPEN → event, not silence. |
| Why no RTOS? | Few tasks with ms-level deadlines; a non-blocking millis() super-loop meets them and is simpler to verify. |
| Response time? | Debounce 50 ms + event→ACK (read from log), well under the requirement. |
| Brute-force PIN? | 3 wrong PINs → 60 s lockout on keypad and phone. |
| Limitations | One detector, USB/power bank power (no battery monitoring), no encryption, PIN hard-coded, no SMS/cloud alert. |
| Future work | Multiple detectors, ESP-NOW encryption, battery voltage reporting, PIN change menu, GSM/app notifications, enclosure PCB. |
