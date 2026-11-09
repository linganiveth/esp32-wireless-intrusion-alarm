#!/usr/bin/env python3
"""Response-time statistics from a detector serial log.

The detector prints one line per acknowledged event, for example:
    [  123456 ms] ACK  DOOR_OPEN   seq=41  event->ACK 3 ms (tries 1)

Usage:
    python tools/latency_from_log.py docs/06_testing/logs/<detector_log>.txt [--debounce 50]

Prints count, average, minimum and maximum event->ACK time per event type and
overall, plus the worst-case door-to-siren time (debounce + event->ACK), which is
checked against NFR1 (< 1000 ms).
"""
import argparse
import re
import statistics
import sys

LINE = re.compile(r"ACK\s+(\w+)\s+seq=(\d+)\s+event->ACK\s+(\d+)\s+ms\s+\(tries\s+(\d+)\)")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("log", help="detector serial log file")
    ap.add_argument("--debounce", type=int, default=50, help="software debounce time in ms (default 50)")
    args = ap.parse_args()

    rows = []
    with open(args.log, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = LINE.search(line)
            if m:
                rows.append((m.group(1), int(m.group(2)), int(m.group(3)), int(m.group(4))))
    if not rows:
        print("No 'event->ACK' lines found. Use the detector's serial log from firmware/detector_node.")
        return 1

    def stats(name, vals, tries):
        retried = sum(1 for t in tries if t > 1)
        print(f"{name:<14} n={len(vals):3d}  avg={statistics.mean(vals):6.1f} ms  "
              f"min={min(vals):4d} ms  max={max(vals):4d} ms  retried={retried}")

    print(f"File: {args.log}\n")
    for t in sorted({r[0] for r in rows}):
        sub = [r for r in rows if r[0] == t]
        stats(t, [r[2] for r in sub], [r[3] for r in sub])
    allv = [r[2] for r in rows]
    stats("ALL", allv, [r[3] for r in rows])
    worst = args.debounce + max(allv)
    print(f"\nWorst-case door-to-siren time = {args.debounce} ms debounce + {max(allv)} ms = {worst} ms "
          f"-> NFR1 (< 1000 ms): {'PASS' if worst < 1000 else 'FAIL'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
