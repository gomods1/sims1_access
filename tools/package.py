"""Builds a release of the mod and the offline installer that carries it.

    python tools/package.py v0.1.0 [--no-build] [--no-installer]

1. mod\\build.bat (release files in mod\\out; no deploy, so no dev.txt)
2. mod\\out\\SimsAccess-<tag>.zip, laid out like the game folder:
       version.dll                  (loader, next to Sims.exe)
       SimsAccess\\SimsAccess.dll    (core)
       SimsAccess\\SpeechHost.exe + prism.dll
       SimsAccess\\docs\\<tag>.html    (the manual in every language, tools/build_docs.py)
       SimsAccess\\licenses\\...      (MinHook, Prism and its parts)
   and mod\\out\\SimsAccess-<tag>.map, the core's map for crash offsets of that release.
3. ..\\installer\\build_offline_sims.bat with that zip:
   ..\\installer\\SimsAccessibilityInstaller-<tag>-offline.exe
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "mod" / "out"
# The release workflow downloads Prism and sets PRISM (mod\build.bat reads it too).
PRISM = Path(os.environ.get("PRISM", r"C:\Users\Nitropc\code\libs\prism\prism-windows-x64"))
INSTALLER = ROOT.parent / "installer"

FILES = [  # (source, path in the zip)
    (OUT / "version.dll", "version.dll"),
    (OUT / "SimsAccess.dll", "SimsAccess/SimsAccess.dll"),
    (OUT / "SpeechHost.exe", "SimsAccess/SpeechHost.exe"),
    (OUT / "prism.dll", "SimsAccess/prism.dll"),
    (ROOT / "mod" / "third_party" / "minhook" / "LICENSE.txt", "SimsAccess/licenses/MinHook.txt"),
    (PRISM / "NOTICE", "SimsAccess/licenses/Prism/NOTICE"),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("tag", help="release tag, e.g. v0.1.0")
    ap.add_argument("--no-build", action="store_true", help="package mod\\out as it is")
    ap.add_argument("--no-installer", action="store_true", help="only make the zip")
    args = ap.parse_args()
    if not re.fullmatch(r"v\d+\.\d+\.\d+", args.tag):
        sys.exit("tag must look like v1.2.3")

    # The manual must say what the mod does in every language: a translation whose sections,
    # tables, lists or key combinations differ from en.md stops the release.
    subprocess.run([sys.executable, str(ROOT / "tools" / "build_docs.py"), "--check"], check=True)
    if not args.no_build:
        subprocess.run(["cmd", "/c", str(ROOT / "mod" / "build.bat")], check=True)

    files = list(FILES)
    for f in sorted((ROOT / "mod" / "sounds").glob("*.wav")):  # the cursors' sound cues
        files.append((f, "SimsAccess/sounds/" + f.name))
    for f in sorted((OUT / "docs").glob("*.html")):  # the manual, one page per language
        files.append((f, "SimsAccess/docs/" + f.name))
    for f in sorted((PRISM / "LICENSES").rglob("*")):
        if f.is_file():
            files.append((f, "SimsAccess/licenses/Prism/" + f.relative_to(PRISM / "LICENSES").as_posix()))
    missing = [str(src) for src, _ in files if not src.is_file()]
    if missing:
        sys.exit("missing: " + ", ".join(missing))

    zip_path = OUT / f"SimsAccess-{args.tag}.zip"
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
        for src, name in files:
            z.write(src, name)
    shutil.copyfile(OUT / "SimsAccess.map", OUT / f"SimsAccess-{args.tag}.map")
    print(f"{zip_path} ({zip_path.stat().st_size:,} bytes, {len(files)} files)")

    if not args.no_installer:
        subprocess.run(["cmd", "/c", str(INSTALLER / "build_offline_sims.bat"), str(zip_path), args.tag],
                       check=True)


if __name__ == "__main__":
    main()
