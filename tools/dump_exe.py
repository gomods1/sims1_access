"""Build an analysable copy of Sims.exe from the running game (SteamStub decrypts .text in memory).

Output: dumps/Sims_<version>_<sha8>_dump.exe, a PE at the preferred base 0x400000 in which:
  - .text is the decrypted code read from the live process, with the loader's relocations undone
    (value -= runtime_base - preferred_base for every HIGHLOW fixup inside .text), so addresses
    match the on-disk .rdata/.data and the VAs quoted in CLAUDE.md;
  - every other section is the on-disk bytes (unencrypted, un-relocated, initial .data values);
  - the import directory is the on-disk one, so Ghidra resolves API names.
For local analysis only: never commit or distribute the output (dumps/ is gitignored).

  python dump_exe.py
"""

import hashlib
import math
import os
import sys
from collections import Counter

import pefile
import pymem

GAME_EXE = r"S:\steam\steamapps\common\The Sims Legacy Collection\Sims.exe"
OUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "dumps")


def entropy(b: bytes) -> float:
    c = Counter(b)
    n = len(b)
    return -sum(v / n * math.log2(v / n) for v in c.values())


def main() -> int:
    disk = open(GAME_EXE, "rb").read()
    sha = hashlib.sha256(disk).hexdigest()
    pe = pefile.PE(data=disk)
    preferred = pe.OPTIONAL_HEADER.ImageBase

    pm = pymem.Pymem("Sims.exe")
    base = pm.process_base.lpBaseOfDll
    delta = (base - preferred) & 0xFFFFFFFF
    print(f"runtime base {base:#x}, preferred {preferred:#x}, delta {delta:#x}")

    text = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    size = min(text.Misc_VirtualSize, text.SizeOfRawData)
    code = bytearray(pm.read_bytes(base + text.VirtualAddress, size))
    print(f".text entropy on disk {entropy(text.get_data()[:size]):.2f}, in memory {entropy(code):.2f}")

    # Undo loader relocations inside .text.
    pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_BASERELOC"]])
    undone = 0
    lo, hi = text.VirtualAddress, text.VirtualAddress + size
    for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []):
        for entry in block.entries:
            if entry.type != pefile.RELOCATION_TYPE["IMAGE_REL_BASED_HIGHLOW"]:
                continue
            if lo <= entry.rva and entry.rva + 4 <= hi:
                off = entry.rva - lo
                val = int.from_bytes(code[off : off + 4], "little")
                code[off : off + 4] = ((val - delta) & 0xFFFFFFFF).to_bytes(4, "little")
                undone += 1
    print(f"undid {undone} relocations in .text")

    # Sanity check: un-relocated absolute pointers should land inside the image at the preferred base.
    image_end = preferred + pe.OPTIONAL_HEADER.SizeOfImage
    sample = [e for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
              if e.type == 3 and lo <= e.rva < hi][:2000]
    inside = sum(
        preferred <= int.from_bytes(code[e.rva - lo : e.rva - lo + 4], "little") < image_end for e in sample
    )
    print(f"relocated pointers inside image after undo: {inside}/{len(sample)}")

    out = bytearray(disk)
    out[text.PointerToRawData : text.PointerToRawData + size] = code

    version = "unknown"
    for fi in getattr(pe, "FileInfo", []) or []:
        for x in fi:
            for st in getattr(x, "StringTable", []):
                version = st.entries.get(b"FileVersion", b"unknown").decode()
    os.makedirs(OUT_DIR, exist_ok=True)
    path = os.path.join(OUT_DIR, f"Sims_{version}_{sha[:8]}_dump.exe")
    open(path, "wb").write(out)
    print("wrote", path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
