"""Print a game string in every language the game ships, found by its US English text.

The mod's own strings name game controls, Sims and interactions ("press Zoom in", "Bob", "Take a
Shower"); each translation must use the label the game shows in that language. This finds the
English entry in the game's string tables (UIText.iff, Live.iff, Build.iff) and prints the entry
at the same index in every language. Some tables are not aligned across languages (UIText STR#
166, the help topics, is shifted in most of them): check that the results agree with each other.

Usage:
  python tools/game_terms.py [--contains] [--objects] <english text> [...]
    --contains  match entries containing the text (default: the whole entry, case-insensitive)
    --objects   also search the object files (GameData/Objects/Objects.far: interaction names,
                catalog names, the tutorial's STR# 301)
"""

import struct
import sys
from collections import defaultdict

import simsfmt

GAME = r"S:\steam\steamapps\common\The Sims Legacy Collection"
# UI text; the live and build panels keep their own strings (the Sim panel tabs are in Live.iff).
UI_FILES = [GAME + r"\GameData\UIText.iff", GAME + r"\GameData\Live.iff", GAME + r"\GameData\Build.iff"]
OBJECTS = GAME + r"\GameData\Objects\Objects.far"


def string_tables(path: str):
    """(file name, chunk, {language: [text, ...]}) for every language-tagged string chunk."""
    for name, data in simsfmt._iter_iffs(path):
        for chunk in simsfmt.read_iff(data):
            if chunk.type not in simsfmt.STRING_CHUNKS:
                continue
            try:
                entries = simsfmt.read_str(chunk.data)
            except (ValueError, struct.error, IndexError):  # unknown format, or a truncated chunk
                continue
            by_language = defaultdict(list)
            for language, text, _comment in entries:
                by_language[language].append(text)
            if 1 in by_language:
                yield name, chunk, by_language


def matches(entry: str, wanted: str, contains: bool) -> bool:
    entry, wanted = entry.strip().lower(), wanted.strip().lower()
    return wanted in entry if contains else entry == wanted


def main(argv: list[str]) -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    contains = "--contains" in argv
    paths = UI_FILES + ([OBJECTS] if "--objects" in argv else [])
    wanted = [a for a in argv if not a.startswith("--")]
    if not wanted:
        print(__doc__)
        return 1
    for path in paths:
        for name, chunk, by_language in string_tables(path):
            for index, english in enumerate(by_language[1]):
                if not any(matches(english, w, contains) for w in wanted):
                    continue
                print(f"== {name} {chunk.type}#{chunk.id} [{chunk.label}] entry {index}")
                for language in sorted(by_language):
                    texts = by_language[language]
                    if language == 0 or index >= len(texts):
                        continue
                    text = texts[index].replace("\r\n", " / ").replace("\n", " / ")
                    print(f"  {language:>2} {simsfmt.LANGUAGE_NAMES.get(language, '?'):<19} {text}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
