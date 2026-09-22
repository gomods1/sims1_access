"""Start The Sims Legacy Collection in a chosen language.

  python tools/launch.py <language> [--restart] [extra switches...]
    language    en, en-gb, fr, de, it, es, nl, da, sv, no, fi, pt, ja, pl, zh-hans, zh-hant, th, ko
    --restart   close the running game first (unsaved progress is lost)
  Example: python tools/launch.py fr

Starts Sims.exe directly with the game's own language switch (-lFrench...), which wins over the
language set in Steam and in the registry for this run only; nothing is changed permanently.
Starting the exe directly avoids the confirmation Steam shows for launch options. Steam must be
running. -skip_intro is added.
"""

import os
import subprocess
import sys

from game import EXE_NAME, find_pid

GAME_DIR = r"S:\steam\steamapps\common\The Sims Legacy Collection"

# Language code -> the name the game's -l<Name> switch takes (case-sensitive; CLAUDE.md "Languages").
LANGUAGES = {
    "en": "USEnglish",
    "en-gb": "UKEnglish",
    "fr": "French",
    "de": "German",
    "it": "Italian",
    "es": "Spanish",
    "nl": "Dutch",
    "da": "Danish",
    "sv": "Swedish",
    "no": "Norwegian",
    "fi": "Finnish",
    "pt": "Portuguese",
    "ja": "Japanese",
    "pl": "Polish",
    "zh-hans": "SimplifiedChinese",
    "zh-hant": "TraditionalChinese",
    "th": "Thai",
    "ko": "Korean",
}


def main(argv: list[str]) -> int:
    if not argv or argv[0].lower() not in LANGUAGES:
        print(__doc__)
        return 1
    name = LANGUAGES[argv[0].lower()]
    extra = [a for a in argv[1:] if a != "--restart"]
    pid = find_pid()
    if pid and "--restart" not in argv:
        print(f"The game is already running (pid {pid}). Close it first, or add --restart.")
        return 1
    if pid:
        subprocess.run(["taskkill", "/PID", str(pid), "/F"], capture_output=True)
        print(f"closed the running game (pid {pid})")
    subprocess.Popen([os.path.join(GAME_DIR, EXE_NAME), f"-l{name}", "-skip_intro", *extra], cwd=GAME_DIR)
    print(f"starting The Sims in {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
