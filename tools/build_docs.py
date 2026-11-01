"""Builds the player's manual: docs/manual/<tag>.md -> mod/out/docs/<tag>.html (pandoc).

    python tools/build_docs.py [--check]

English (en.md) is the source; every other file is a translation of it in the game's language
whose tag names it (the tags util::DocTag() returns: the mod opens SimsAccess\\docs\\<tag>.html
for the game's language with Control+Shift+F1, en.html when it is missing). Each page is
self-contained (the stylesheet is embedded), with a table of contents.

--check only compares the translations with en.md: the same headings at each level, the same
table rows and list items, and in each table row's first cell the same key combinations (count
of "+", function keys), so a section, a row or a changed key left out of a translation shows.

Keep it from drifting: a change to the mod's keys or features edits en.md and every translation
in the same change, along with the Keys* strings Control+F1 speaks (all languages). Translations
write key combinations with an ASCII "+" ("Ctrl+Shift+F1", "Strg+Umschalt+F1").
"""
import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "docs" / "manual"
OUT = ROOT / "mod" / "out" / "docs"

# The tags of mod/core/util/language.cpp (DocTag), one per language the mod speaks.
TAGS = ["en", "fr", "de", "it", "es", "nl", "da", "sv", "no", "fi", "pt", "ja", "pl",
        "zh-hans", "zh-hant", "th", "ko"]


def shape(text: str) -> list[str]:
    """What must line up between the English and a translation: heading levels, table rows,
    list items (one character each, in order)."""
    out = []
    in_front = False
    for line in text.splitlines():
        if line.strip() == "---":
            in_front = not in_front
            continue
        if in_front:
            continue
        if m := re.match(r"(#+) ", line):
            out.append("h" + str(len(m.group(1))))
        elif line.startswith("|") and not re.match(r"\|[-| :]+\|$", line):
            # a key table's row: its key cell's combinations look alike in every language (the
            # same number of "+" and the same function keys), whatever the keys are called
            key = line.split("|")[1]
            out.append("r" + str(key.count("+")) + "".join(re.findall(r"\bF\d{1,2}\b", key)))
        elif re.match(r"(\d+\.|-) ", line):
            out.append("l")
    return out


def check() -> int:
    english = shape((SRC / "en.md").read_text(encoding="utf-8"))
    bad = 0
    for tag in TAGS[1:]:
        path = SRC / f"{tag}.md"
        if not path.exists():
            print(f"{tag}: missing")
            bad += 1
            continue
        other = shape(path.read_text(encoding="utf-8"))
        if other != english:
            at = next((i for i, (a, b) in enumerate(zip(english, other)) if a != b), min(len(english), len(other)))
            print(f"{tag}: differs from en.md at element {at} "
                  f"(en {english[at:at + 3]}, {tag} {other[at:at + 3]}; {len(english)} vs {len(other)} elements)")
            bad += 1
    print("manual translations: " + ("all match en.md" if not bad else f"{bad} to fix"))
    return 1 if bad else 0


def build() -> int:
    pandoc = shutil.which("pandoc")
    if not pandoc:
        sys.exit("pandoc not found (https://pandoc.org)")
    OUT.mkdir(parents=True, exist_ok=True)
    built = 0
    for tag in TAGS:
        src = SRC / f"{tag}.md"
        if not src.exists():
            print(f"note: no {src.name}; the game falls back to English for it")
            continue
        subprocess.run([pandoc, str(src), "--from", "markdown", "--to", "html5", "--standalone",
                        "--embed-resources", "--css", str(SRC / "style.css"), "--toc", "--toc-depth=2",
                        "--output", str(OUT / f"{tag}.html")], check=True)
        built += 1
    print(f"manual: {built} pages in {OUT}")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true", help="compare the translations with en.md")
    args = ap.parse_args()
    sys.exit(check() if args.check else build())


if __name__ == "__main__":
    main()
