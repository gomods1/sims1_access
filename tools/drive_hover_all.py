# Needs frida/virtual_mouse.js. Hover each neighborhood top-bar button in turn (game window at 0,0).
import time

for x in (695, 768, 914, 1018, 1123, 1226, 1331, 1435, 1540, 1645, 1748):
    rpc.set_pos(x - 10, 230)  # noqa: F821  (injected by frida_run.py)
    time.sleep(0.3)
    rpc.set_pos(x, 224)  # noqa: F821
    time.sleep(2.0)
rpc.release()  # noqa: F821
