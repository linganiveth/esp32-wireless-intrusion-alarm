#!/usr/bin/env python3
"""Copy firmware/common/packet.h into every sketch folder that uses it.

The Arduino IDE only compiles files that are inside the sketch folder, so the
detector and the hub each keep a copy of the shared header. Edit only
firmware/common/packet.h, then run:

    python tools/sync_packet_h.py          # copy
    python tools/sync_packet_h.py --check  # only report differences (exit 1 if any)
"""
import argparse
import filecmp
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MASTER = ROOT / "firmware" / "common" / "packet.h"
TARGETS = [ROOT / "firmware" / "detector_node" / "packet.h", ROOT / "firmware" / "hub" / "packet.h"]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true", help="only check, do not copy")
    args = ap.parse_args()
    differ = [t for t in TARGETS if not t.exists() or not filecmp.cmp(MASTER, t, shallow=False)]
    if args.check:
        for t in differ:
            print("out of date:", t.relative_to(ROOT))
        print("all copies match" if not differ else f"{len(differ)} copy/copies differ")
        return 1 if differ else 0
    for t in differ:
        shutil.copyfile(MASTER, t)
        print("updated:", t.relative_to(ROOT))
    if not differ:
        print("all copies already match")
    return 0


if __name__ == "__main__":
    sys.exit(main())
