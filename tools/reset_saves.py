r"""Put the game's stock neighborhoods back (the families as shipped), keeping a backup of the
current saves.

  python tools/reset_saves.py [--only UserData,UserData2] [--label text] [--no-backup]
                              [--reset-options] [--list] [--restore <backup folder>]

    (no options)     back up every neighborhood in Saved Games, then replace each one with the
                     stock copy from the game's install folder (only the ones that differ)
    --only a,b       reset only these neighborhoods (UserData, UserData2 ... UserData8)
    --label text     name for the backup folder: sims-backups/<date>-<text> (default "saves")
    --no-backup      skip the backup
    --reset-options  also replace UserData\Options.iff (the game's option sliders); kept by default
    --list           show which neighborhoods differ from stock and exit
    --restore path   copy a backup folder back into Saved Games (the reverse operation),
                     after backing up what is there

Backups go to C:\Users\Nitropc\code\mods\sims-backups (next to this repo, not inside it).
The game must not be running: it writes the saves on exit. The install folder's UserData* are
the untouched first copy (verified: a save only writes the Saved Games copy).
"""

import argparse
import filecmp
import os
import shutil
import subprocess
import sys
from datetime import datetime

GAME = r"S:\steam\steamapps\common\The Sims Legacy Collection"
SAVES = os.path.join(os.environ["USERPROFILE"], r"Saved Games\Electronic Arts\The Sims 25")
BACKUPS = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), os.pardir, "sims-backups")
BACKUPS = os.path.normpath(BACKUPS)
NEIGHBORHOODS = ["UserData"] + ["UserData%d" % n for n in range(2, 9)]
OPTIONS = "Options.iff"  # only in UserData; the user's own settings


def game_running():
    out = subprocess.run(["tasklist", "/FI", "IMAGENAME eq Sims.exe", "/NH"],
                         capture_output=True, text=True).stdout
    return "Sims.exe" in out


def differs(a, b):
    """True when folder a is not file-for-file identical to folder b (Options.iff ignored)."""
    cmp = filecmp.dircmp(a, b, ignore=[OPTIONS])
    stack = [cmp]
    while stack:
        c = stack.pop()
        if c.left_only or c.right_only or c.diff_files or c.funny_files:
            return True
        stack.extend(c.subdirs.values())
    return False


def copy_tree(src, dst):
    if os.path.exists(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def backup(names, label):
    stamp = datetime.now().strftime("%Y-%m-%d-%H%M")
    dest = os.path.join(BACKUPS, "%s-%s" % (stamp, label))
    os.makedirs(dest)
    for n in names:
        copy_tree(os.path.join(SAVES, n), os.path.join(dest, n))
    print("backed up %s to %s" % (", ".join(names), dest))
    return dest


def main():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument("--only")
    ap.add_argument("--label", default="saves")
    ap.add_argument("--no-backup", action="store_true")
    ap.add_argument("--reset-options", action="store_true")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--restore")
    ap.add_argument("-h", "--help", action="store_true")
    args = ap.parse_args()
    if args.help:
        print(__doc__)
        return 0

    names = args.only.split(",") if args.only else NEIGHBORHOODS
    for n in names:
        if n not in NEIGHBORHOODS:
            sys.exit("unknown neighborhood %s (UserData, UserData2 .. UserData8)" % n)

    if args.list:
        for n in names:
            changed = differs(os.path.join(SAVES, n), os.path.join(GAME, n))
            print("%-10s %s" % (n, "changed" if changed else "stock"))
        return 0

    if game_running():
        sys.exit("Sims.exe is running: close the game first (it writes the saves on exit)")

    if args.restore:
        src = args.restore
        if not os.path.isdir(src):
            src = os.path.join(BACKUPS, args.restore)
        if not os.path.isdir(src):
            sys.exit("no such backup folder: %s" % args.restore)
        present = [n for n in names if os.path.isdir(os.path.join(src, n))]
        if not present:
            sys.exit("the backup holds none of %s" % ", ".join(names))
        if not args.no_backup:
            backup(present, "before-restore")
        for n in present:
            copy_tree(os.path.join(src, n), os.path.join(SAVES, n))
            print("restored %s from %s" % (n, src))
        return 0

    changed = [n for n in names if differs(os.path.join(SAVES, n), os.path.join(GAME, n))]
    if not changed:
        print("nothing to do: %s already match the stock saves" % ", ".join(names))
        return 0
    if not args.no_backup:
        backup(changed, args.label)
    for n in changed:
        keep = None
        opt = os.path.join(SAVES, n, OPTIONS)
        if not args.reset_options and os.path.isfile(opt):
            with open(opt, "rb") as f:
                keep = f.read()
        copy_tree(os.path.join(GAME, n), os.path.join(SAVES, n))
        if keep is not None:
            with open(opt, "wb") as f:
                f.write(keep)
        print("reset %s to the stock neighborhood%s" % (n, "" if keep is None else " (options kept)"))
    print("current neighborhood in the registry stays as it is (SIMS_CURRENT_NEIGHBORHOOD_PATH)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
