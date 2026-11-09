# Stage 1: Requirements analysis

What the system must do, without deciding how. Every requirement has an ID that is traced to a test in [`../07_integration_validation`](../07_integration_validation).

## Use cases

| ID | Use case | Actor | Description |
|---|---|---|---|
| UC1 | Arm | Occupant | Enter PIN and arm; leave within the exit delay |
| UC2 | Enter and disarm | Occupant | Open the door and enter the PIN within the entry delay |
| UC3 | Intrusion | Intruder | Motion or door opening while armed raises the alarm |
| UC4 | Tamper | Intruder | Opening the detector enclosure raises the alarm in any state |
| UC5 | Sensor lost | System | Detector loses power or range; the hub shows a fault |
| UC6 | Remote control | Occupant | Check status and arm/disarm from a phone on the hub's Wi-Fi |

## Functional requirements

| ID | Requirement |
|---|---|
| FR1 | Detect motion in the protected room |
| FR2 | Detect the door opening and closing |
| FR3 | Detect tampering with the detector enclosure in every state, including disarmed |
| FR4 | Arm and disarm with a 4-digit PIN from a keypad and from a phone |
| FR5 | Exit delay of 30 s after arming; entry delay of 15 s after the door opens |
| FR6 | Sound a siren on alarm; show status on LEDs, a display and the phone page |
| FR7 | Report a detector that has stopped communicating |
| FR8 | Lock out PIN entry after 3 consecutive wrong PINs |

## Non-functional requirements

| ID | Requirement | Rationale |
|---|---|---|
| NFR1 | Sensor event → siren on in under 1 s | React before an intruder reaches the hub |
| NFR2 | No event lost; every event acknowledged | A missed door or motion event defeats the alarm |
| NFR3 | A cut wire or dead detector never fails silently | Fail-safe behaviour is the core of a security product |
| NFR4 | Indoor radio range of at least 10 m | Covers a typical flat from one hub position |
| NFR5 | Works without internet or a Wi-Fi router | Alarm must work during a network outage |

## Constraints

- Two ESP32 DevKit V1 boards, no extra radio module; breadboard-level, low-cost modules.
- Arduino framework (C++); bare-metal super-loop, no application RTOS tasks.
- About five working days for design, build, test and documentation.
