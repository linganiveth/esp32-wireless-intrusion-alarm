# Lesson 01: ESP32 setup and board identity

Design-flow stage: Software implementation (toolchain) · Day plan: 09:00-09:30

## 1. Concept

An ESP32 DevKit V1 is four things on one board:

| Part | What it does | Why it matters here |
|---|---|---|
| ESP32-WROOM-32 module | Dual-core 240 MHz processor, 520 KB SRAM, 4 MB flash, Wi-Fi + Bluetooth radio | Runs the firmware; the radio does ESP-NOW and the phone web page |
| USB-to-UART chip (CP2102 or CH340) | Turns USB into a serial port (COMx on Windows) | Flashing and Serial Monitor; needs a driver |
| 3.3 V regulator (AMS1117) | Makes 3.3 V from USB 5 V | ESP32 pins are 3.3 V only, never feed them 5 V |
| EN and BOOT buttons | EN = reset; BOOT holds GPIO0 low | GPIO0 low at reset = "download mode" for flashing |

**How flashing works.** At reset the ESP32 reads GPIO0. High = run your program; low = wait for new firmware over UART. The board's auto-reset circuit uses the USB chip's DTR/RTS lines to do this for you. If it fails, you hold BOOT by hand.

**Why the MAC address.** Every ESP32 has a unique 6-byte address burned into eFuse at the factory. ESP-NOW sends packets to a MAC address, so the detector must know the hub's MAC and vice versa. Today we record both.

## 2. Build step

1. Arduino IDE 2.x -> File -> Preferences -> Additional boards manager URLs:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. Tools -> Board -> Boards Manager -> install **esp32 by Espressif Systems**.
3. Plug in one ESP32 with a data cable. Check Device Manager -> Ports for `Silicon Labs CP210x` or `USB-SERIAL CH340` (COMx). No port = install the CP210x or CH340 driver.
4. Tools -> Board -> esp32 -> **ESP32 Dev Module**; Tools -> Port -> that COMx.
5. Open `firmware/test_sketches/L01_board_check/L01_board_check.ino`, Upload.
6. Serial Monitor -> 115200 baud.
7. Repeat for the second board. Put a tape label on each: **HUB** and **DET**.

## 3. Check

Serial Monitor every 5 s (values vary by board; the MAC is unique):
```
=== L01 board check ===
Chip   : ESP32-D0WD-V3 rev 3, 2 cores, 240 MHz
Flash  : 4 MB
STA MAC: 24:6F:28:AA:BB:CC
C array: {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC}
LED    : blinking on GPIO2
```
Pass: report prints on both boards, LED blinks at 1 Hz, two different MACs.

Record them:

| Board | STA MAC | C array |
|---|---|---|
| HUB | | |
| DET | | |

## 4. Common mistakes

| Symptom | Cause | Fix |
|---|---|---|
| No COM port | Charge-only cable or no driver | Use a data cable; install CP210x / CH340 driver |
| `Failed to connect ... Wrong boot mode` | Auto-reset did not enter download mode | Hold BOOT, click Upload, release when "Connecting..." shows |
| Garbage characters | Serial Monitor baud wrong | Set 115200 |
| Upload errors at high speed | Weak cable / USB hub | Tools -> Upload Speed -> 115200 |
| LED does not blink, report prints | LED on a different pin on your board | Fine; the report is what matters |
