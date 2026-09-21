# Needs frida/virtual_mouse.js. Hover the neighborhood "Families" button and screenshot.
import subprocess
import sys
import time

GAME_PY = [sys.executable, __file__.replace("drive_tooltip_test.py", "game.py")]

rpc.set_pos(900, 230)  # noqa: F821  (injected by frida_run.py)
time.sleep(0.5)
rpc.set_pos(914, 224)  # noqa: F821
time.sleep(3)
subprocess.run(GAME_PY + ["shot", "04_virtual_hover"])
