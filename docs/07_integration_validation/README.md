# Stages 6 and 7: System integration and validation

## Integration procedure

1. **Pairing.** Each sketch holds the peer's MAC address ([`../02_specification/board_ids.md`](../02_specification/board_ids.md)); both radios are fixed to Wi-Fi channel 1. The hub's access point also uses channel 1.
2. **Mounting.** Detector box above the door: PIR on top facing the room, reed switch on the frame with the magnet on the door (gap ≤ 10 mm), tamper lever pressed by the closed lid.
3. **Going wireless.** Detector on a USB power bank, hub on USB; the radio is the only link.
4. **End-to-end check.** Hub log shows `Detector ONLINE`, then the initial door and lid states; detector log shows `ACK … tries 1` for each event.

If the detector logs `SEND FAILED` for every event, change `HUB_MAC` to the hub's **AP MAC** (printed at start-up, usually STA MAC + 1) and re-flash the detector.

## Response time (NFR1)

The detector prints, for every event, the time from queuing the event to receiving the hub's ACK:

```
[  123456 ms] ACK  DOOR_OPEN   seq=41  event->ACK 3 ms (tries 1)
```

The hub switches the siren on before it sends the ACK, so

`t_response = 50 ms (debounce) + event->ACK`

Collect about 20 door events, save the detector log to `../06_testing/logs/`, then run:

```
python tools/latency_from_log.py docs/06_testing/logs/<detector_log>.txt
```

| Quantity | Value |
|---|---|
| Radio round trip (HT6 ping test) | 2.4–3.0 ms typical, 28.8 ms max |
| event→ACK average / max | fill in |
| Door-to-siren max (requirement < 1000 ms) | fill in |

## Range (NFR4)

Hub fixed on a desk; at each position open and close the door 10 times and record what the hub receives.

| Position | Events received (of 10) | Retries | Offline seen? |
|---|---|---|---|
| 1 m, line of sight | | | |
| 5 m, line of sight | | | |
| 10 m, line of sight | | | |
| 10 m, through one wall | | | |

## Validation: requirement-to-test traceability

| Requirement | Verified by | Result |
|---|---|---|
| FR1 Motion detection | ST5, HT2 | |
| FR2 Door open / close | ST1, ST3 | |
| FR3 Tamper in any state | ST6 | |
| FR4 PIN arm / disarm (keypad + phone) | ST2, ST10 | |
| FR5 Exit and entry delays | ST2, ST3, ST4 | |
| FR6 Siren and status display | ST4, HT4, HT5 | |
| FR7 Offline detector reported | ST7 | |
| FR8 PIN lockout | ST8 | |
| NFR1 Event → siren < 1 s | Response-time test above | |
| NFR2 No lost events | ST1, ST11, HT6 | |
| NFR3 Fail-safe | ST7 + switch-to-GND wiring | |
| NFR4 Range ≥ 10 m | Range test above | |
| NFR5 No internet needed | ST10 (hub's own Wi-Fi) | Pass |

## Demonstration scenarios

| # | Scenario | Expected result |
|---|---|---|
| 1 | Status at a glance | OLED and phone show DISARMED, detector online |
| 2 | Arm with exit delay | PIN + A → 30 s countdown with chirps → ARMED |
| 3 | Authorised entry | Door opens → 15 s → PIN + # → DISARMED, no siren |
| 4 | Intruder | Motion while armed → instant ALARM; disarm from the phone |
| 5 | Tamper | Lid opened while disarmed → ALARM |
| 6 | Sensor lost | Detector unplugged → OFFLINE within 15 s |

Demo script and likely questions: [`../slides/demo_day_plan.md`](../slides/demo_day_plan.md).
