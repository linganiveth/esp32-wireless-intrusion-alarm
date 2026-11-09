# ESP-NOW packet protocol (v1)

Source of truth: `firmware/common/packet.h`. Copy it into every sketch folder when it changes.

**Packet (12 bytes, packed, little-endian)**

| Offset | Field | Type | Meaning |
|---|---|---|---|
| 0 | node_id | uint8 | 0 = HUB, 1 = DETECTOR |
| 1 | type | uint8 | see the table below |
| 2 | seq | uint16 | increments per new event; a retry keeps the same seq; an ACK echoes it |
| 4 | battery_mV | uint16 | reserved (0 on USB power) |
| 6 | uptime_ms | uint32 | sender millis() |
| 10 | crc | uint16 | CRC-16/CCITT (poly 0x1021, init 0xFFFF) over bytes 0–9 |

| type | Name | Sender |
|---|---|---|
| 0 | HEARTBEAT | DET, every 5 s |
| 1 | MOTION | DET |
| 2 | DOOR_OPEN | DET |
| 3 | DOOR_CLOSED | DET |
| 4 | TAMPER | DET (lid opened) |
| 5 | ACK | HUB |
| 6 | TAMPER_CLEAR | DET (lid closed) |

**Timing:** ACK timeout 100 ms, max 3 tries; hub marks the detector OFFLINE after 15 s of silence. Channel 1.
**FSM timings:** exit 30 s, entry 15 s, siren max 3 min, 3 wrong PINs → 60 s lockout.
