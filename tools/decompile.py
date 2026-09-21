"""Decompile functions (by preferred-base VA) from the Ghidra project into re/decomp/.

  python decompile.py <va> [va ...]      e.g. python decompile.py 724210 702afa

Each address may be anywhere inside a function. Output files are re/decomp/<entry>_<name>.c.
"""

import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
JAVA_HOME = r"C:\Users\Nitropc\scoop\apps\temurin21-jdk\current"
HEADLESS = r"C:\Users\Nitropc\scoop\apps\ghidra\current\support\analyzeHeadless.bat"


def main(argv: list[str]) -> int:
    if not argv:
        print(__doc__)
        return 1
    env = dict(os.environ, JAVA_HOME=JAVA_HOME, PATH=JAVA_HOME + r"\bin;" + os.environ["PATH"])
    out = os.path.join(ROOT, "re", "decomp")
    cmd = [HEADLESS, os.path.join(ROOT, "re", "ghidra"), "SimsRE", "-process", "-noanalysis", "-readOnly",
           "-scriptPath", os.path.join(ROOT, "tools", "ghidra"), "-postScript", "Decompile.java", out, *argv]
    res = subprocess.run(cmd, env=env, capture_output=True, text=True)
    for line in res.stdout.splitlines():
        if "Decompile.java>" in line:
            print(line.split("Decompile.java>", 1)[1].strip())
    if res.returncode != 0:
        print(res.stdout[-3000:], res.stderr[-2000:])
    return res.returncode


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
