"""Readers for The Sims 1 data containers (FAR archives, IFF resource files, STR# string tables).

Layouts verified against the Legacy Collection install (build 1.10.7):
  FAR:  "FAR!byAZ", u32 version (=1), u32 manifest offset; manifest = u32 count, then per entry
        u32 raw size, u32 stored size, u32 offset, u32 name length, name bytes.
  IFF:  60-byte text header ("IFF FILE 2.5:TYPE FOLLOWED BY SIZE..."), u32 BE rsmp offset, then chunks:
        4-byte type, u32 BE size (includes the 76-byte chunk header), u16 BE id, u16 BE flags,
        64-byte NUL-padded label, data.
STR# format codes come from community docs (FreeSO's STR.cs); treat them as leads until checked
against every chunk (see `strings` below, which reports unknown codes instead of guessing).

CLI:
  python simsfmt.py far <file.far>                 list archive entries
  python simsfmt.py iff <file.iff|file.far>        list chunks (for a FAR: every .iff inside)
  python simsfmt.py strings <file.iff> [lang]      dump all STR#/CTSS/TTAs text (lang 1 = US English)
"""

import struct
import sys
from dataclasses import dataclass

IFF_MAGIC = b"IFF FILE 2.5:TYPE FOLLOWED BY SIZE"
FAR_MAGIC = b"FAR!byAZ"
CHUNK_HEADER = 76

# Code page of the text in each language, by the language byte of STR# entries (also the game's
# own language number, [[0xa67044] + 0x68]). Verified on UIText.iff: Polish decodes with its
# letters, Japanese to kana/kanji, Chinese to hanzi, Korean to hangul, Thai to Thai script (whose
# words are joined by '_'). 12 Hebrew and 13 Russian hold English placeholders.
LANGUAGE_NAMES = {
    1: "US English", 2: "UK English", 3: "French", 4: "German", 5: "Italian", 6: "Spanish",
    7: "Dutch", 8: "Danish", 9: "Swedish", 10: "Norwegian", 11: "Finnish", 12: "Hebrew",
    13: "Russian", 14: "Portuguese (Brazil)", 15: "Japanese", 16: "Polish",
    17: "Simplified Chinese", 18: "Traditional Chinese", 19: "Thai", 20: "Korean",
}
LANGUAGE_ENCODINGS = {15: "cp932", 16: "cp1250", 17: "cp936", 18: "cp950", 19: "cp874", 20: "cp949"}
TEXT_ENCODING = "cp1252"  # every other language, and untagged strings


def encoding_of(language: int) -> str:
    return LANGUAGE_ENCODINGS.get(language, TEXT_ENCODING)


@dataclass
class FarEntry:
    name: str
    offset: int
    size: int
    stored_size: int


def read_far(data: bytes) -> list[FarEntry]:
    if data[:8] != FAR_MAGIC:
        raise ValueError("not a FAR archive")
    version, manifest = struct.unpack_from("<II", data, 8)
    if version != 1:
        raise ValueError(f"unsupported FAR version {version}")
    (count,) = struct.unpack_from("<I", data, manifest)
    pos = manifest + 4
    entries = []
    for _ in range(count):
        raw, stored, offset, name_len = struct.unpack_from("<IIII", data, pos)
        pos += 16
        name = data[pos : pos + name_len].decode(TEXT_ENCODING)
        pos += name_len
        entries.append(FarEntry(name, offset, raw, stored))
    return entries


def far_file(data: bytes, entry: FarEntry) -> bytes:
    if entry.size != entry.stored_size:
        raise ValueError(f"{entry.name}: compressed FAR entries are not supported")
    return data[entry.offset : entry.offset + entry.size]


@dataclass
class Chunk:
    type: str
    id: int
    flags: int
    label: str
    data: bytes


def read_iff(data: bytes) -> list[Chunk]:
    if not data.startswith(IFF_MAGIC):
        raise ValueError("not an IFF file")
    pos = 64
    chunks = []
    while pos + CHUNK_HEADER <= len(data):
        ctype = data[pos : pos + 4].decode("latin-1")
        size, cid, flags = struct.unpack_from(">IHH", data, pos + 4)
        if size < CHUNK_HEADER or pos + size > len(data):
            raise ValueError(f"bad chunk size {size} at {pos:#x}")
        label = data[pos + 12 : pos + CHUNK_HEADER].split(b"\0", 1)[0].decode(TEXT_ENCODING)
        chunks.append(Chunk(ctype, cid, flags, label, data[pos + CHUNK_HEADER : pos + size]))
        pos += size
    return chunks


def _cstr(data: bytes, pos: int, encoding: str = TEXT_ENCODING) -> tuple[str, int]:
    end = data.index(b"\0", pos)
    return data[pos:end].decode(encoding, "replace"), end + 1


def _pstr(data: bytes, pos: int, encoding: str = TEXT_ENCODING) -> tuple[str, int]:
    n = data[pos]
    return data[pos + 1 : pos + 1 + n].decode(encoding, "replace"), pos + 1 + n


def read_str(chunk_data: bytes) -> list[tuple[int, str, str]]:
    """Parse a STR#-family chunk into (language, value, comment) tuples.

    Language 0 means "not language tagged"; each entry is decoded with its language's code page.
    Raises ValueError on an unknown format code.
    """
    (fmt,) = struct.unpack_from("<h", chunk_data, 0)
    out: list[tuple[int, str, str]] = []
    if fmt == 0:  # Pascal strings
        (count,) = struct.unpack_from("<H", chunk_data, 2)
        pos = 4
        for _ in range(count):
            s, pos = _pstr(chunk_data, pos)
            out.append((0, s, ""))
    elif fmt == -1:  # NUL-terminated strings
        (count,) = struct.unpack_from("<H", chunk_data, 2)
        pos = 4
        for _ in range(count):
            s, pos = _cstr(chunk_data, pos)
            out.append((0, s, ""))
    elif fmt == -2:  # NUL-terminated value/comment pairs
        (count,) = struct.unpack_from("<H", chunk_data, 2)
        pos = 4
        for _ in range(count):
            s, pos = _cstr(chunk_data, pos)
            c, pos = _cstr(chunk_data, pos)
            out.append((0, s, c))
    elif fmt == -3:  # language-tagged value/comment pairs
        (count,) = struct.unpack_from("<H", chunk_data, 2)
        pos = 4
        for _ in range(count):
            lang = chunk_data[pos]
            s, pos = _cstr(chunk_data, pos + 1, encoding_of(lang))
            c, pos = _cstr(chunk_data, pos, encoding_of(lang))
            out.append((lang, s, c))
    elif fmt == -4:  # language sets, each a list of Pascal value/comment pairs
        sets = chunk_data[2]
        pos = 3
        for _ in range(sets):
            (count,) = struct.unpack_from("<H", chunk_data, pos)
            pos += 2
            for _ in range(count):
                lang = chunk_data[pos]
                s, pos = _pstr(chunk_data, pos + 1, encoding_of(lang))
                c, pos = _pstr(chunk_data, pos, encoding_of(lang))
                out.append((lang, s, c))
    else:
        raise ValueError(f"unknown STR# format {fmt}")
    return out


STRING_CHUNKS = ("STR#", "CTSS", "TTAs", "CST\0")


def _iter_iffs(path: str):
    data = open(path, "rb").read()
    if data.startswith(FAR_MAGIC):
        for e in read_far(data):
            if e.name.lower().endswith(".iff"):
                yield e.name, far_file(data, e)
    else:
        yield path, data


def main(argv: list[str]) -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    if len(argv) < 2:
        print(__doc__)
        return 1
    cmd, path = argv[0], argv[1]
    if cmd == "far":
        for e in read_far(open(path, "rb").read()):
            print(f"{e.size:>10} {e.name}")
    elif cmd == "iff":
        for name, data in _iter_iffs(path):
            print(f"== {name}")
            for c in read_iff(data):
                print(f"  {c.type} #{c.id:<6} flags={c.flags:#06x} {len(c.data):>7}  {c.label}")
    elif cmd == "strings":
        want = int(argv[2]) if len(argv) > 2 else None
        for name, data in _iter_iffs(path):
            for c in read_iff(data):
                if c.type not in STRING_CHUNKS:
                    continue
                try:
                    entries = read_str(c.data)
                except ValueError as ex:
                    print(f"{c.type}#{c.id} [{c.label}] !! {ex}")
                    continue
                for i, (lang, s, comment) in enumerate(entries):
                    if want is not None and lang not in (0, want):
                        continue
                    note = f"  // {comment}" if comment else ""
                    print(f"{c.type}#{c.id} [{c.label}] {i} L{lang}: {s!r}{note}")
    else:
        print(__doc__)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
