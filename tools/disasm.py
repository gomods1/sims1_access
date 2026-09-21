"""Disassemble the decrypted dump at a preferred-base VA (fast; no Ghidra needed).

  python disasm.py <va> [count=60]        e.g. python disasm.py 6f0e40 80
  python disasm.py <va> --func            until the first ret/int3 run (one function)

Uses dumps/Sims_1.10.7_b7c8765e_dump.exe (tools/dump_exe.py). Annotates call/jmp targets with
vtable slots (re/vtables.tsv) and addresses of known strings (re/string_xrefs.tsv).
"""

import csv
import os
import sys

import capstone
import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DUMP = os.path.join(ROOT, "dumps", "Sims_1.10.7_b7c8765e_dump.exe")

_pe = None
_names = None


def pe():
    global _pe
    if _pe is None:
        _pe = pefile.PE(DUMP, fast_load=True)
    return _pe


def read(va: int, size: int) -> bytes:
    p = pe()
    rva = va - p.OPTIONAL_HEADER.ImageBase
    return p.get_data(rva, size)


def names() -> dict[int, str]:
    global _names
    if _names is None:
        _names = {}
        vt = os.path.join(ROOT, "re", "vtables.tsv")
        if os.path.exists(vt):
            with open(vt, encoding="utf-8") as f:
                for row in csv.DictReader(f, delimiter="\t"):
                    a = int(row["target_va"], 16)
                    _names.setdefault(a, f"{row['class']}::slot{row['slot']}")
        extra = os.path.join(ROOT, "re", "names.csv")
        if os.path.exists(extra):
            with open(extra, encoding="utf-8") as f:
                for row in csv.DictReader(f):
                    _names[int(row["va"], 16)] = row["name"]
    return _names


def disasm(va: int, count: int = 60, func: bool = False) -> list[str]:
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    code = read(va, 0x4000)
    out = []
    n = names()
    for ins in md.disasm(code, va):
        note = ""
        if ins.mnemonic in ("call", "jmp") and ins.op_str.startswith("0x"):
            t = int(ins.op_str, 16)
            if t in n:
                note = "  ; " + n[t]
        out.append(f"{ins.address:08x}  {ins.bytes.hex():<20} {ins.mnemonic} {ins.op_str}{note}")
        if func and ins.mnemonic in ("ret", "int3"):
            break
        if not func and len(out) >= count:
            break
    return out


def main(argv: list[str]) -> int:
    if not argv:
        print(__doc__)
        return 1
    va = int(argv[0], 16)
    func = "--func" in argv
    rest = [a for a in argv[1:] if not a.startswith("--")]
    count = int(rest[0]) if rest else 60
    print("\n".join(disasm(va, count, func)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
