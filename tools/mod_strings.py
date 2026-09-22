"""The mod's own strings: check the translations, and move them to and from JSON for translating.

English is the source: mod/core/util/strings_table.h, X(Id, "English") entries under /* group */
comments. Each translation is mod/core/lang/<language>.cpp, {Str::Id, "text"} entries in the same
order and groups; this tool writes those files, so they all look the same.

Usage:
  python tools/mod_strings.py check                    every translation against the English table
  python tools/mod_strings.py export <language>        JSON to stdout: [{id, group, english, text}]
  python tools/mod_strings.py write <language> <json>  lang/<language>.cpp from {id: text} (or the
                                                       export format); missing ids are left out
Languages: french german italian spanish dutch danish swedish norwegian finnish portuguese japanese
polish simplified_chinese traditional_chinese thai korean
"""

import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TABLE = os.path.join(ROOT, "mod", "core", "util", "strings_table.h")
LANG_DIR = os.path.join(ROOT, "mod", "core", "lang")

# file stem: (name in the file's comment, table name in lang/translations.h, game language numbers)
LANGUAGES = {
    "french": ("French", "kFrench", "3"),
    "german": ("German", "kGerman", "4"),
    "italian": ("Italian", "kItalian", "5"),
    "spanish": ("Spanish (Spain)", "kSpanish", "6"),
    "dutch": ("Dutch", "kDutch", "7"),
    "danish": ("Danish", "kDanish", "8"),
    "swedish": ("Swedish", "kSwedish", "9"),
    "norwegian": ("Norwegian", "kNorwegian", "10"),
    "finnish": ("Finnish", "kFinnish", "11"),
    "portuguese": ("Portuguese (Brazil)", "kPortuguese", "14"),
    "japanese": ("Japanese", "kJapanese", "15"),
    "polish": ("Polish", "kPolish", "16"),
    "simplified_chinese": ("Simplified Chinese", "kSimplifiedChinese", "17"),
    "traditional_chinese": ("Traditional Chinese", "kTraditionalChinese", "18"),
    "thai": ("Thai", "kThai", "19"),
    "korean": ("Korean", "kKorean", "20"),
}

LITERAL = re.compile(r'"((?:[^"\\]|\\.)*)"')
LINE_WIDTH = 116


def unescape(body: str) -> str:
    return re.sub(r"\\(.)", lambda m: {"n": "\n", "t": "\t"}.get(m.group(1), m.group(1)), body)


def escape(text: str) -> str:
    return text.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")


def literals(source: str) -> str:
    """The text of a run of adjacent C string literals."""
    return "".join(unescape(m.group(1)) for m in LITERAL.finditer(source))


def read_english() -> list[dict]:
    """[{id, group, english}] in table order."""
    text = open(TABLE, encoding="utf-8").read()
    body = text[text.index("#define SA_STRINGS(X)") :]
    entries, group = [], ""
    # A group comment, or an entry: X(Id, "..." "...") with the literals over several lines.
    for m in re.finditer(r'/\*\s*(.*?)\s*\*/|\bX\((\w+),((?:\s*\\?\s*"(?:[^"\\]|\\.)*")+)\s*\\?\s*\)', body):
        if m.group(1) is not None:
            group = m.group(1)
        else:
            entries.append({"id": m.group(2), "group": group, "english": literals(m.group(3))})
    return entries


def read_translation(language: str) -> dict[str, str]:
    path = os.path.join(LANG_DIR, language + ".cpp")
    if not os.path.exists(path):
        return {}
    text = open(path, encoding="utf-8").read()
    out = {}
    for m in re.finditer(r'\{Str::(\w+),((?:\s*"(?:[^"\\]|\\.)*")+)\s*\}', text):
        if m.group(1) in out:
            raise ValueError(f"{language}: {m.group(1)} appears twice")
        out[m.group(1)] = literals(m.group(2))
    return out


def placeholders(text: str) -> list[str]:
    """printf conversions in argument order; positional ones (%2$s) are put in their place."""
    found = re.findall(r"%(?:(\d+)\$)?[-+ #0]*\d*(?:\.\d+)?([a-zA-Z%])", text)
    found = [(pos, conv) for pos, conv in found if conv != "%"]
    if any(pos for pos, _ in found):
        return [conv for _, conv in sorted(found, key=lambda f: int(f[0] or 0))]
    return [conv for _, conv in found]


def check() -> int:
    english = read_english()
    ids = {e["id"]: e for e in english}
    problems = 0
    for language in LANGUAGES:
        try:
            translation = read_translation(language)
        except ValueError as ex:
            print(ex)
            problems += 1
            continue
        if not translation:
            print(f"{language}: no file")
            problems += 1
            continue
        missing = [e["id"] for e in english if e["id"] not in translation]
        unknown = [i for i in translation if i not in ids]
        wrong = [i for i, t in translation.items() if i in ids and placeholders(t) != placeholders(ids[i]["english"])]
        for label, items in (("missing", missing), ("unknown ids", unknown), ("placeholders differ", wrong)):
            if items:
                print(f"{language}: {label}: {', '.join(items)}")
                problems += 1
        if not (missing or unknown or wrong):
            print(f"{language}: {len(translation)} strings, ok")
    return 1 if problems else 0


def export(language: str) -> int:
    translation = read_translation(language)
    rows = [dict(e, text=translation.get(e["id"], "")) for e in read_english()]
    sys.stdout.write(json.dumps(rows, ensure_ascii=False, indent=1) + "\n")
    return 0


def wrap(text: str, first_width: int, width: int) -> list[str]:
    """Pieces of `text`, each short enough for its line: broken after a space where possible,
    anywhere in scripts without spaces (Chinese, Japanese, Thai)."""
    pieces, rest, limit = [], text, first_width
    while len(escape(rest)) > limit:
        cut = limit
        while len(escape(rest[:cut])) > limit:
            cut -= 1
        space = rest.rfind(" ", 0, cut)
        if space > cut // 2:
            cut = space + 1
        pieces.append(rest[:cut])
        rest, limit = rest[cut:], width
    pieces.append(rest)
    return pieces


def entry_lines(string_id: str, text: str) -> list[str]:
    head = f"    {{Str::{string_id}, "
    one_line = f'{head}"{escape(text)}"}},'
    if len(one_line) <= LINE_WIDTH:
        return [one_line]
    indent = "     "
    pieces = wrap(text, LINE_WIDTH - len(indent) - 4, LINE_WIDTH - len(indent) - 4)
    lines = [f"    {{Str::{string_id},"]
    lines += [f'{indent}"{escape(p)}"' for p in pieces]
    lines[-1] += "},"
    return lines


def write(language: str, json_path: str) -> int:
    name, table, numbers = LANGUAGES[language]
    data = json.load(open(json_path, encoding="utf-8"))
    texts = {r["id"]: r["text"] for r in data} if isinstance(data, list) else data
    out = [
        f"// {name}: the game's language {numbers}. Written by tools/mod_strings.py from a translation of",
        "// util/strings_table.h; keep the order and groups of the English table.",
        '#include "translations.h"',
        "",
        "#include <iterator>",
        "",
        "namespace lang {",
        "namespace {",
        "",
        "const Translation kEntries[] = {",
    ]
    group = None
    for e in read_english():
        text = texts.get(e["id"], "")
        if not text:
            continue
        if e["group"] != group:
            group = e["group"]
            out.append(f"    // {group}")
        out += entry_lines(e["id"], text)
    out += [
        "};",
        "",
        "}  // namespace",
        "",
        f"extern const Translations {table} = {{kEntries, std::size(kEntries)}};",
        "",
        "}  // namespace lang",
        "",
    ]
    path = os.path.join(LANG_DIR, language + ".cpp")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))
    print(f"wrote {path}")
    return 0


def main(argv: list[str]) -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    if argv[:1] == ["check"]:
        return check()
    if len(argv) == 2 and argv[0] == "export" and argv[1] in LANGUAGES:
        return export(argv[1])
    if len(argv) == 3 and argv[0] == "write" and argv[1] in LANGUAGES:
        return write(argv[1], argv[2])
    print(__doc__)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
