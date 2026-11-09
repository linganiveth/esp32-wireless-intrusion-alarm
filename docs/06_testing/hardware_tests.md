# Hardware tests (HT1-HT6)

| ID | Test | Pass criterion | Measured | Result | Date |
|---|---|---|---|---|---|
| HT1 | Power rails | 5.0 ± 0.25 V, 3.3 ± 0.1 V | | | |
| HT2 | PIR output | HIGH ~3.3 V on movement, LOW ~0 V still; detects at 4 m | 5/5 walk-pasts detected; OUT HIGH 1.0-2.5 s per event. 1/2/4 m range and OUT voltage not yet measured | Pass (detection); range pending | 2026-11-01 |
| HT3 | Reed and tamper inputs | Door shut 0 V / open 3.3 V; lid on 0 V / off 3.3 V | Debounce: one event per action for both inputs. Reed 1 raw edge/action; tamper microswitch 1-31 raw edges/action (heavy lever bounce). Voltages: pending | Pass (logic), voltages pending | 2026-11-01 |
| HT4 | Siren driver | Buzzer sounds; collector ≤ 0.2 V | | | |
| HT5 | LEDs, keypad, OLED | All LEDs; 16 keys; OLED at 0x3C | LEDs: all 4 (READY 26, ARMED 27, ALARM 14, FAULT 13) pass (user-reported 2026-11-05). Keypad 16 keys and OLED 0x3C: pending | Pass (LEDs); keypad/OLED pending | 2026-11-05 |
| HT6 | Radio link | ≥ 99 of 100 pings answered | Same desk: 0 losses in 452 consecutive pings (#359-#810); RTT typ. 2.4-3.0 ms, max 28.8 ms. 10 m / wall test pending | Pass (desk) | 2026-11-01 |
