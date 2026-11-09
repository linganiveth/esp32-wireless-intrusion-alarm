# Lesson 07–10 (merged): Complete firmware — protocol, alarm FSM, supervision, phone page

**Design-flow stage:** SW design → SW implementation → SW unit test (feeds System Integration).
**Files:** `firmware/detector_node/` (detector_node.ino, packet.h, debounced_input.h) and `firmware/hub/` (hub.ino, packet.h).

---

## 1. Concepts (read once, then build)

### 1.1 Why a packet "protocol" at all?
In L02 the detector said PING and the hub said PONG. A real alarm must also answer these questions:

| Question | Answer in our packet | Field |
|---|---|---|
| Who sent it? | node ID (0 = hub, 1 = detector) | `node_id` |
| What happened? | MOTION / DOOR_OPEN / DOOR_CLOSED / TAMPER / TAMPER_CLEAR / HEARTBEAT / ACK | `type` |
| Is it the same message again? | sequence number | `seq` |
| Was it corrupted on the way? | CRC-16/CCITT over the first 10 bytes | `crc` |
| Is the detector still alive? | heartbeat every 5 s, plus `uptime_ms` | `uptime_ms` |

The struct is `__attribute__((packed))`, so both boards see exactly 12 bytes with no padding.

### 1.2 ACK + retry (reliability)
Radio packets can be lost. The detector keeps every event until the hub ACKs it.

```
Detector                         Hub
  TX MOTION seq=41  ───────────▶  CRC ok → state change → siren ON
                    ◀───────────  ACK seq=41
  (no ACK in 100 ms? resend, max 3 tries, then "SEND FAILED")
```
* The hub sends the ACK **after** acting on the event, so an ACK means "already handled".
* If the ACK is lost, the detector resends. The hub sees the same `seq` again and **re-ACKs without acting twice** (duplicate filter).

### 1.3 Supervision (fail-safe)
If someone smashes or unplugs the detector, it doesn't send anything. That's why silence itself has to raise an alarm:
* The detector sends a HEARTBEAT every **5 s**.
* If the hub hears nothing for **15 s** (3 missed heartbeats), it shows **DETECTOR OFFLINE**: the FAULT LED turns on and the buzzer chirps every 10 s.

### 1.4 The alarm state machine (FSM)

```
              PIN + A (lid closed, detector online)
  DISARMED ───────────────────────────────▶ EXIT_DELAY (30 s, chirp each second)
     ▲  ▲                                         │ time over
     │  │ PIN + #                                 ▼
     │  └──────────────── any state ◀──────── ARMED
     │                                    door │      │ motion
     │                                         ▼      │
     │                            ENTRY_DELAY (15 s)  │
     │                                 │ time over    │
     │                                 ▼              ▼
     └──────── PIN + # ──────────── ALARM ◀──────────┘
                                     ▲
                 TAMPER ─────────────┘  (from ANY state, even DISARMED)
```
* The siren is limited to **3 min**. After that it stops, but the state stays ALARM until a valid PIN is entered.
* **3 wrong PINs** lock the keypad and the phone page for **60 s**.

### 1.5 Non-blocking super-loop
There is no `delay()` in `loop()`. Every job checks `millis()` and returns quickly:

```
loop():  drain RX queue → keypad → tick() timers → outputs → OLED (every 250 ms) → web server
```
The ESP-NOW receive callback runs in the Wi-Fi task. It **only copies the packet into a queue** and returns; all decisions are made in `loop()` (the same idea as the PIR ISR in L04).

### 1.6 Wi-Fi AP + ESP-NOW together
The hub runs `WIFI_AP_STA`. Its access point **"HomeGuard-Hub"** is on **channel 1**, the same channel the detector uses. Your phone joins that Wi-Fi and opens `http://192.168.4.1`.

---

## 2. Build steps

**Libraries** (Arduino IDE → Library Manager), already installed in L05: *Adafruit SSD1306*, *Adafruit GFX*, *Keypad* (Mark Stanley).

1. **Flash the detector (Board 2, MAC D0:EF:76:33:EE:44)**
   1. Open `firmware/detector_node/detector_node.ino`. All 3 files should show as tabs.
   2. Select Board: ESP32 Dev Module and the detector's COM port, then Upload.
   3. Open Serial Monitor at 115200.
2. **Flash the hub (Board 1, MAC FC:E8:C0:75:C1:94)**
   1. Open `firmware/hub/hub.ino`, select the hub's COM port, then Upload.
   2. Open a second Serial Monitor window, or use PuTTY for the second port.
3. **Set the detector's PIR jumper to H** (repeat trigger) and turn its *time* pot fully anticlockwise (shortest).
4. **Phone:** connect Wi-Fi to `HomeGuard-Hub`, password `homeguard123`, then browse to `192.168.4.1`. Ignore any "no internet" warning.

> If you change the PIN, change `PIN_CODE` in hub.ino. It is "1234" by default.

---

## 3. Check: expected outputs

### 3.1 Start-up
**Hub serial monitor**
```
HUB ready. Wi-Fi 'HomeGuard-Hub' (password homeguard123), open http://192.168.4.1
STA MAC FC:E8:C0:75:C1:94  AP MAC FC:E8:C0:75:C1:95  channel 1  OLED OK
[     5xx ms] Hub started, DISARMED
[    xxxx ms] Detector ONLINE
[    xxxx ms] EVENT DOOR_CLOSED (seq 1)
[    xxxx ms] EVENT TAMPER_CLEAR (seq 2)        ← or TAMPER if the lid is open
```
**Detector serial monitor**
```
DETECTOR ready on channel 1. Door CLOSED, lid CLOSED. PIR warming up 30 s
[  xxxx ms] TX   DOOR_CLOSED seq=1
[  xxxx ms] ACK  DOOR_CLOSED seq=1  event->ACK 3 ms (tries 1)
...
[ 30xxx ms] PIR ready
```
**OLED:** `DISARMED`, `Last: ...`, `Detector: OK 2s`, `PIN:`. **LEDs:** READY (green) on.

### 3.2 Software tests (record the results in `docs/06_testing/software_tests.md`)

| ID | Do this | Pass if |
|---|---|---|
| ST1 Protocol | Open and close the door 5 times | 10 events on the hub, each with an `ACK ... tries 1` on the detector; *event→ACK* < 50 ms |
| ST2 Arm/exit delay | Enter `1 2 3 4 A` | EXIT DELAY, a 30 s countdown on the OLED, the ARMED LED blinks, a chirp each second, then ARMED (solid) |
| ST3 Entry path | While ARMED, open the door, then type `1234#` within 15 s | ENTRY DELAY, then DISARMED, with no siren |
| ST4 Entry timeout | While ARMED, open the door and wait 15 s | ALARM: the siren sounds and the ALARM LED turns on |
| ST5 Instant motion | While ARMED, wave at the PIR | ALARM straight away |
| ST6 Tamper 24 h | While DISARMED, open the detector lid | ALARM even though the system is disarmed; `1234#` silences it |
| ST7 Supervision | Unplug the detector's USB | About 15 s later: `DETECTOR OFFLINE`, FAULT LED on, a chirp every 10 s; plug it back in → `Detector ONLINE` |
| ST8 Lockout | Type `0000A` three times | `KEYPAD LOCKED for 60 s`; the OLED shows LOCKED, the FAULT LED blinks, and even the correct PIN is refused for 60 s |
| ST9 Arm refused | Open the lid, then try `1234A` | `Cannot arm: detector lid open` |
| ST10 Phone | Arm from the phone, then disarm from the phone | The page shows the state change within 2 s; the event list matches the serial log |
| ST11 Retry | Hold the detector far away or behind metal and trigger the door | You may see `RETRY` lines; the hub still logs the event exactly once (duplicate filter) |

Paste both serial logs into `docs/06_testing/logs/L07_firmware_<date>.txt`.

---

## 4. Common mistakes

| Symptom | Likely cause | Fix |
|---|---|---|
| Detector shows `SEND FAILED` every time, hub silent | The hub's radio isn't on channel 1, or it uses the AP interface for ESP-NOW | Check that the hub prints `channel 1`. If it still fails, change `HUB_MAC` in detector_node.ino to the hub's **AP MAC** (printed at start-up, usually STA MAC + 1) and re-flash the detector |
| `'packet.h' not found` | The sketch folder is missing its copy | Every sketch folder must have its own packet.h (copied from `firmware/common/`) |
| Detector goes OFFLINE every few seconds | Heartbeats are lost, or the hub is busy | Check the RTT from L02. Reduce the distance first |
| Siren never stops | The state is ALARM and nobody entered `1234#` | The siren stops after 3 min, but the **state** stays ALARM until a valid PIN is entered. That's intended |
| Phone page doesn't load | The phone switched to mobile data | Turn mobile data off, or choose "stay connected" when the phone warns about no internet |
| OLED says nothing, but serial says `OLED NOT FOUND` | Wiring | Repeat the L05 I2C scan |
| PIR spams MOTION | H jumper or time pot | The detector already sends motion at most once per rise; set the time pot to minimum |

---

## 5. Results (fill in)

| Test | Result | Notes |
|---|---|---|
| ST1 | | event→ACK typical ___ ms, max ___ ms |
| ST2 | | |
| ST3 | | |
| ST4 | | |
| ST5 | | |
| ST6 | | |
| ST7 | | OFFLINE after ___ s |
| ST8 | | |
| ST9 | | |
| ST10 | | |
| ST11 | | |
