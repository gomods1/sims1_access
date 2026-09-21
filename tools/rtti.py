"""Recover MSVC x86 RTTI (classes, base classes, vtables) from the Sims.exe dump.

Layouts (MSVC, 32-bit, all pointers are absolute VAs at the preferred base):
  TypeDescriptor           { void* pVFTable; void* spare; char name[] }   name = ".?AVClass@@"
  CompleteObjectLocator    { u32 signature(0); u32 offset; u32 cdOffset; TypeDescriptor*; ClassHierarchyDescriptor* }
  ClassHierarchyDescriptor { u32 signature; u32 attributes; u32 numBaseClasses; BaseClassDescriptor** }
  BaseClassDescriptor      { TypeDescriptor*; u32 numContainedBases; i32 mdisp, pdisp, vdisp; u32 attributes }
  vtable: the dword just before slot 0 points to the CompleteObjectLocator.

Writes re/classes.tsv and re/vtables.tsv (function names joined from re/functions.tsv).

  python rtti.py [dump.exe]
"""

import csv
import glob
import os
import re
import struct
import sys

import pefile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RE_DIR = os.path.join(ROOT, "re")


def demangle(name: str) -> str:
    """.?AVcTSWin@@ -> cTSWin ; .?AVFoo@Bar@@ -> Bar::Foo (templates left mangled)."""
    body = name[4:] if name.startswith((".?AV", ".?AU")) else name
    body = body[:-2] if body.endswith("@@") else body
    if body.startswith("?$"):
        return body
    return "::".join(reversed([p for p in body.split("@") if p]))


def main(argv: list[str]) -> int:
    dump = argv[0] if argv else sorted(glob.glob(os.path.join(ROOT, "dumps", "*_dump.exe")))[-1]
    pe = pefile.PE(dump, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    image = pe.get_memory_mapped_image()

    def u32(va: int) -> int:
        return struct.unpack_from("<I", image, va - base)[0]

    sec = {s.Name.rstrip(b"\0").decode(): (base + s.VirtualAddress, base + s.VirtualAddress + s.Misc_VirtualSize)
           for s in pe.sections}
    text_lo, text_hi = sec[".text"]
    rd_lo, rd_hi = sec[".rdata"]

    def in_text(v: int) -> bool:
        return text_lo <= v < text_hi

    # 1. Type descriptors: every ".?AV"/".?AU" string; descriptor starts 8 bytes earlier.
    tds = {}
    for m in re.finditer(rb"\.\?A[VU][\x21-\x7e]+?@@\x00", image):
        va = base + m.start() - 8
        tds[va] = m.group()[:-1].decode()

    # 2. Scan .rdata dwords once: complete object locators and a reverse index of pointer values.
    rd = image[rd_lo - base : rd_hi - base]
    dwords = struct.unpack_from(f"<{len(rd) // 4}I", rd)
    def valid(v: int) -> bool:
        return base <= v < base + len(image) - 16

    def plausible_chd(chd: int) -> bool:
        return valid(chd) and u32(chd) == 0 and 1 <= u32(chd + 8) <= 64 and valid(u32(chd + 12))

    cols = {}
    for i in range(len(dwords) - 4):
        if dwords[i] == 0 and dwords[i + 3] in tds and dwords[i + 1] < 0x10000 and plausible_chd(dwords[i + 4]):
            cols[rd_lo + i * 4] = (dwords[i + 1], dwords[i + 3], dwords[i + 4])  # offset, td, chd
    refs_to_col = {}
    for i, v in enumerate(dwords):
        if v in cols:
            refs_to_col.setdefault(v, []).append(rd_lo + i * 4)

    # 3. Function names from Ghidra's export (optional).
    fnames = {}
    fpath = os.path.join(RE_DIR, "functions.tsv")
    if os.path.exists(fpath):
        with open(fpath, encoding="utf-8") as f:
            for row in csv.DictReader(f, delimiter="\t"):
                fnames[int(row["va"], 16)] = row["name"]

    classes, vt_rows = [], []
    for col, (offset, td, chd) in sorted(cols.items()):
        cls = demangle(tds[td])
        n_bases = u32(chd + 8)
        arr = u32(chd + 12)
        bases = []
        for k in range(1, n_bases):  # entry 0 is the class itself
            bcd = u32(arr + 4 * k)
            btd = u32(bcd)
            bases.append(f"{demangle(tds.get(btd, hex(btd)))}@{struct.unpack_from('<i', image, bcd + 8 - base)[0]}")
        for ref in refs_to_col.get(col, []):
            vt = ref + 4
            if not in_text(u32(vt)):
                continue
            slot = 0
            while True:
                # Ends at the first non-code pointer, e.g. the next vtable's COL pointer (.rdata).
                target = u32(vt + 4 * slot)
                if not in_text(target):
                    break
                vt_rows.append((cls, f"{vt:08x}", slot, f"{target:08x}", fnames.get(target, "")))
                slot += 1
            classes.append((cls, f"{vt:08x}", offset, slot, " ".join(bases), tds[td]))

    with open(os.path.join(RE_DIR, "classes.tsv"), "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["class", "vtable_va", "this_offset", "slots", "bases(name@mdisp)", "mangled"])
        w.writerows(classes)
    with open(os.path.join(RE_DIR, "vtables.tsv"), "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["class", "vtable_va", "slot", "target_va", "target_name"])
        w.writerows(vt_rows)
    print(f"{len(tds)} type descriptors, {len(cols)} object locators, {len(classes)} vtables, {len(vt_rows)} slots")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
