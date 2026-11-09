# Tools

| Script | What it does | Usage |
|---|---|---|
| [`sync_packet_h.py`](sync_packet_h.py) | Copies the master `firmware/common/packet.h` into the detector and hub sketch folders (Arduino only compiles files inside the sketch folder) | `python tools/sync_packet_h.py` (`--check` to only compare) |
| [`latency_from_log.py`](latency_from_log.py) | Reads a detector serial log and prints avg/min/max `event->ACK` time per event type and the worst-case door-to-siren time against NFR1 | `python tools/latency_from_log.py docs/06_testing/logs/<log>.txt` |
| [`gen_schematics.py`](gen_schematics.py) | Draws the hub and detector schematics with schemdraw | `cd hardware/schematics && python ../../tools/gen_schematics.py` |

Python 3.9 or newer. Only `gen_schematics.py` needs extra packages: `pip install -r tools/requirements.txt`.
