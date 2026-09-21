"""Trace every virtual method of one or more classes in the running game.

  python trace_class.py <Class[,Class2]> [seconds] [--with other.js] [--drive drive.py]

Builds TRACE_TARGETS from re/vtables.tsv (unique target functions, labelled Class::slotN(va)),
prepends it to frida/trace_class.js and runs it through frida_run.py. Pass --with to combine
another script (e.g. frida/virtual_mouse.js) for driving.
"""

import csv
import json
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
VTABLES = os.path.join(os.path.dirname(HERE), "re", "vtables.tsv")


def main(argv: list[str]) -> int:
    if not argv:
        print(__doc__)
        return 1
    wanted = set(argv[0].split(","))
    rest = argv[1:]
    extra = []
    if "--with" in rest:
        i = rest.index("--with")
        extra = rest[i + 1].split(",")
        rest = rest[:i] + rest[i + 2:]

    targets, seen = [], set()
    with open(VTABLES, encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            if row["class"] in wanted and row["target_va"] not in seen:
                seen.add(row["target_va"])
                targets.append(["0x" + row["target_va"], f"{row['class']}::slot{row['slot']}({row['target_va']})"])
    if not targets:
        raise SystemExit(f"no vtable rows for {wanted}")

    body = open(os.path.join(HERE, "frida", "trace_class.js"), encoding="utf-8").read()
    tmp = tempfile.NamedTemporaryFile("w", suffix=".js", delete=False, encoding="utf-8")
    tmp.write(f"const TRACE_TARGETS = {json.dumps(targets)};\n{body}")
    tmp.close()
    scripts = ",".join([tmp.name] + [os.path.join(HERE, e) for e in extra])
    try:
        return subprocess.call([sys.executable, os.path.join(HERE, "frida_run.py"), scripts] + rest)
    finally:
        os.unlink(tmp.name)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
