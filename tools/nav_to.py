"""Press a key until the mod speaks an item whose name (before the first ", " or ". ") contains the
given text (reads SimsAccess.log).

  python nav_to.py <key> <text> [max=60]      e.g. python nav_to.py right Atrás
Prints the matching line; exits 1 when not found. Keys as in keys.py.
"""
import os
import subprocess
import sys
import time

LOG = r"S:\steam\steamapps\common\The Sims Legacy Collection\SimsAccess\SimsAccess.log"
HERE = os.path.dirname(os.path.abspath(__file__))


def last_spoken() -> str:
    with open(LOG, encoding="utf-8", errors="replace") as f:
        lines = [l for l in f.read().splitlines() if "Spoke:" in l]
    return lines[-1].split("Spoke:", 1)[1].strip() if lines else ""


def main(argv):
    key, text = argv[0], argv[1]
    limit = int(argv[2]) if len(argv) > 2 else 60
    previous = last_spoken()
    for _ in range(limit):
        subprocess.run([sys.executable, os.path.join(HERE, "keys.py"), key])
        time.sleep(0.45)
        line = last_spoken()
        label = line.split(". ", 1)[0].split(", ", 1)[0]  # the item's name, not its description
        if text.lower() in label.lower():
            print(line)
            return 0
        if line == previous:
            break  # the end of the group repeats the item: stop instead of repeating it
        previous = line
    print("not found; last:", last_spoken())
    return 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main(sys.argv[1:]))
