# Software tests (ST) — firmware v1 (Lesson 07)

Procedures: see `docs/learning/07_firmware_protocol_fsm.md` section 3.2. Logs go in `logs/L07_firmware_<date>.txt`.

| ID | Requirement checked | Pass criterion | Result | Date | Notes |
|---|---|---|---|---|---|
| ST1 | Reliable event delivery (ACK) | 10/10 events ACKed, event→ACK < 50 ms | | | |
| ST2 | Arm with exit delay | 30 s countdown, then ARMED | | | |
| ST3 | Entry delay + disarm | DISARMED within 15 s, no siren | | | |
| ST4 | Entry delay timeout | ALARM after 15 s | | | |
| ST5 | Instant motion alarm | ALARM immediately | | | |
| ST6 | 24 h tamper | ALARM even while DISARMED | | | |
| ST7 | Detector supervision | OFFLINE ≤ 15 s after power loss, ONLINE on return | | | |
| ST8 | PIN lockout | Locked 60 s after 3 wrong PINs | | | |
| ST9 | Arm interlock | Arm refused while lid open | | | |
| ST10 | Phone page | State and events update ≤ 2 s; arm/disarm works | Page loads at http://192.168.4.1 from a laptop on HomeGuard-Hub (phone not yet tried) | Pass (laptop) | 2026-11-05 | Page refresh changed to skip while the PIN box is in use |
| ST11 | Duplicate filter | Retried event processed once | | | |
