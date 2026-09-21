"""Inspect live Sims.exe memory from Python with class names resolved through RTTI vtables.

  python memtool.py "<python code>"      e.g. python memtool.py "dump(G(0xa7f484), 0x80)"
  python memtool.py -f script.py [--with frida/x.js]

Helpers available to the code:
  va(x)        preferred-base VA -> runtime address        pva(a)  runtime -> preferred VA
  u32(a)       read dword at runtime address a             G(v)    u32(va(v)) (read a global)
  u32s(a, n)   n dwords                                     std(a)  MSVC std::string at a
  cls(a)       RTTI class of the object at a (by vtable)    dump(a, size) annotated dword dump
  rpc          the Frida script's exports (memread.js and any --with scripts)
"""

import csv
import os
import sys

import frida

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PREFERRED = 0x400000

_vtables: dict[int, str] = {}
with open(os.path.join(ROOT, "re", "classes.tsv"), encoding="utf-8") as f:
    for row in csv.DictReader(f, delimiter="\t"):
        off = int(row["this_offset"])
        _vtables[int(row["vtable_va"], 16)] = row["class"] + (f"@{off}" if off else "")


class Mem:
    def __init__(self, extra_scripts: list[str]):
        common = open(os.path.join(HERE, "frida", "common.js"), encoding="utf-8").read()
        parts = [common] + [open(os.path.join(HERE, p), encoding="utf-8").read()
                            for p in ["frida/memread.js"] + extra_scripts]
        self.session = frida.attach("Sims.exe")
        self.script = self.session.create_script("\n".join(parts))
        self.script.on("message", lambda m, d: print(m.get("payload", m), flush=True))
        self.script.load()
        self.rpc = self.script.exports_sync
        self.base = int(self.rpc.base(), 16)

    def close(self):
        self.script.unload()
        self.session.detach()


M: Mem | None = None


def va(x: int) -> int:
    return x - PREFERRED + M.base


def pva(a: int) -> int:
    return a - M.base + PREFERRED


def u32s(a: int, n: int) -> list:
    return M.rpc.read_u32s(a, n)


def u32(a: int):
    r = u32s(a, 1)
    return r[0] if r else None


def G(v: int):
    return u32(va(v))


def std(a: int):
    return M.rpc.read_std(a)


def in_exe(a: int) -> bool:
    return M.base <= a < M.base + 0x800000


def cls(a: int):
    if not a:
        return None
    vt = u32(a)
    if vt is None or not in_exe(vt):
        return None
    return _vtables.get(pva(vt))


def annotate(v) -> str:
    if v is None:
        return "??"
    if in_exe(v):
        name = _vtables.get(pva(v))
        return f"VA {pva(v):#x}" + (f" vtable {name}" if name else "")
    if v > 0x10000:
        c = cls(v)
        if c:
            return f"-> {c}"
    return ""


def dump(a: int, size: int = 0x100):
    words = u32s(a, size // 4)
    for i, w in enumerate(words):
        if w is None:
            print(f"  +{i*4:#05x}  <unreadable>")
            break
        note = annotate(w)
        small = f"{w:d}" if w < 0x10000 else ""
        f = w if w < 0x80000000 else w - 0x100000000
        print(f"  +{i*4:#05x}  {w:08x}  {small:>6}  {note}")


def main(argv: list[str]) -> int:
    global M
    sys.stdout.reconfigure(encoding="utf-8")
    if not argv:
        print(__doc__)
        return 1
    extra = []
    if "--with" in argv:
        i = argv.index("--with")
        extra = argv[i + 1].split(",")
        argv = argv[:i] + argv[i + 2:]
    code = open(argv[1], encoding="utf-8").read() if argv[0] == "-f" else argv[0]
    M = Mem(extra)
    try:
        exec(code, dict(globals(), rpc=M.rpc))
    finally:
        M.close()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
