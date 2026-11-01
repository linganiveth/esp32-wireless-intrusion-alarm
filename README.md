# Wireless Home Security System (25ES604 ESD Term Project)

Ajax-style intrusion alarm built on two ESP32 boards: a **detector node** (PIR motion, door reed switch, tamper switch) reports over **ESP-NOW** to a **hub** (keypad, OLED, siren, LEDs, phone web page) that runs the alarm state machine.

Author: Linganiveth S G (EBS26016) - M.Tech Embedded Systems

## Folder map (follows the course design flow)

| Folder | Design-flow stage | Contents |
|---|---|---|
| `docs/01_requirements/` | Requirements analysis | Numbered FR / TR / DR requirements |
| `docs/02_specification/` | Specification | Timing constants, packet format, pin map |
| `docs/03_architecture/` | System architecture | Block diagram, HW/SW partition |
| `docs/04_hardware/` | Hardware design | Circuit calculations, component selection |
| `docs/05_software/` | Software design | Modules, state machine, main loop |
| `docs/06_testing/` | HW + SW testing | HT1-HT6, ST1-ST6 results and logs |
| `docs/07_integration_validation/` | Integration + validation | Integration steps, T1-T7, traceability |
| `docs/08_operation_maintenance/` | Operation + maintenance | User guide, known limits |
| `docs/learning/` | Lessons | Concept + build step + check for each topic |
| `docs/daily_log/` | Progress | One file per working day + master checklist |
| `docs/report/`, `docs/slides/` | Submission | Final report and presentation |
| `hardware/` | HW implementation | Schematics, BOM, photos |
| `firmware/` | SW implementation | Arduino sketches for both nodes + test sketches |
| `tools/` | Validation | Scripts, e.g. latency from serial logs |

## Toolchain
- Arduino IDE 2.x + ESP32 board package (Espressif), board: ESP32 Dev Module
- Libraries: Keypad, Adafruit SSD1306 + Adafruit GFX (or U8g2), WebServer (built in), esp_now (built in)
