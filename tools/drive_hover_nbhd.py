# Hover the mouse across the neighborhood screen's top button bar so tooltips render.
# Coordinates are screen pixels measured on shots/01_first_launch.png (2560x1600 display).
import ctypes
import time

user32 = ctypes.windll.user32
user32.SetProcessDPIAware()

BAR_Y = 225
for x in (695, 915, 1020, 1120, 1227, 1332, 1435, 1540, 1645, 1750, 740, 770):
    user32.SetCursorPos(x, BAR_Y)
    time.sleep(1.5)
user32.SetCursorPos(1280, 900)
