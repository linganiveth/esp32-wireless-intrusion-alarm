# Lesson 02: ESP-NOW ping between hub and detector

Design-flow stage: System architecture check (is the chosen link workable?) · Day plan: 09:30-10:15

## 1. Concept

**ESP-NOW** is Espressif's peer-to-peer protocol. It sends short Wi-Fi frames straight from one ESP32's MAC address to another, with no router, no IP address and no connection setup.

| Property | Value | Why it suits the alarm |
|---|---|---|
| Addressing | 6-byte MAC of the peer | We recorded both MACs in Lesson 01 |
| Payload | up to 250 bytes per frame | Our event packet is ~12 bytes |
| Latency | a few milliseconds | Far inside the 1 s siren requirement (TR1) |
| Infrastructure | none | Works in the exam hall without Wi-Fi |
| Rule | both boards on the same Wi-Fi channel | Matters in Lesson 10 when the hub also runs a Wi-Fi access point |

**Callbacks.** When a frame arrives, ESP-NOW calls your receive function from the Wi-Fi task, not from loop(). The rule: copy the data, set a flag, return. Do the printing and logic in loop(). This is the same pattern as an interrupt service routine.

**Ping / pong.** The detector sends PING with a sequence number and its send time in microseconds. The hub echoes it back as PONG. When the PONG returns, the detector subtracts the send time from now: that is the **round-trip time (RTT)**. Counting sent vs received gives **packet loss**. These two numbers prove the link before we build on it.

## 2. Build step

1. Flash `firmware/test_sketches/L02_espnow_hub/` to **HUB (Board 1)**.
2. Flash `firmware/test_sketches/L02_espnow_det/` to **DET (Board 2)**.
3. Open two Serial Monitors (Arduino IDE 2: one per board window) at 115200, or use PuTTY for the second port.

## 3. Check

HUB:
```
HUB ready on channel 1, waiting for PING from DET
PING #1 from DET -> PONG sent
PING #2 from DET -> PONG sent
```
DET:
```
DET ready on channel 1, pinging HUB every 1 s
PONG #1  RTT 2.10 ms  (received 1/1)
PONG #2  RTT 1.95 ms  (received 2/2)
...
--- 10 sent, 10 received, loss 0 %, avg RTT 2.0 ms, max 3.1 ms ---
```
Pass: loss 0 % over 30 pings with boards on the same desk; RTT a few ms (exact value varies). Unplug the HUB: DET must print `No PONG for #n` every second, then recover when the HUB is plugged back in.

Record:

| Distance | Sent | Received | Loss % | Avg RTT (ms) | Max RTT (ms) |
|---|---|---|---|---|---|
| Same desk | | | | | |
| ~10 m / through a wall (HT6) | | | | | |

## 4. Common mistakes

| Symptom | Cause | Fix |
|---|---|---|
| Every ping times out | MAC typed wrong or boards swapped | Compare with `docs/02_specification/board_ids.md`; DET sketch on Board 2 |
| `Add peer FAILED` | ESP-NOW not initialised, or peer added twice | Keep the order: WiFi.mode, esp_now_init, add_peer |
| Compile error in `onRecv` | Core 2.x vs 3.x callback signature | Sketch handles both; tell me the exact error text |
| Works, then stops after adding Wi-Fi AP (later) | Channel mismatch | Lesson 10 fixes the channel on both boards |
