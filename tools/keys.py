"""Send key presses to the game window without focusing it (PostMessage WM_KEYDOWN/WM_CHAR/WM_KEYUP).

  python keys.py <key> [key ...] [--delay 0.6]
  keys: up down left right tab enter space esc home end pgup pgdn back comma period, any single character,
        or f1..f12, with optional shift+ / ctrl+ / alt+ prefixes (alt+ posts WM_SYSKEYDOWN/UP, as
        Windows sends Alt combinations). Example: python keys.py tab down down enter

The mod's window subclass sees these exactly like real key presses; the game's own message loop
calls TranslateMessage, so WM_CHAR is generated there (posting it here too would double every
character). The user's real keyboard focus is untouched.
"""

import ctypes
import sys
import time

user32 = ctypes.WinDLL("user32", use_last_error=True)
user32.PostMessageW.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_size_t, ctypes.c_ssize_t]

WM_KEYDOWN, WM_KEYUP, WM_CHAR = 0x100, 0x101, 0x102
WM_SYSKEYDOWN, WM_SYSKEYUP = 0x104, 0x105
ALT_CONTEXT = 1 << 29  # lParam: Alt is down
VK = {"up": 0x26, "down": 0x28, "left": 0x25, "right": 0x27, "tab": 0x09, "enter": 0x0D,
      "space": 0x20, "esc": 0x1B, "home": 0x24, "end": 0x23, "shift": 0x10, "ctrl": 0x11, "back": 0x08, "comma": 0xBC, "period": 0xBE, "minus": 0xBD, "plus": 0xBB,
      "pgup": 0x21, "pgdn": 0x22, "delete": 0x2e, "insert": 0x2d}
CHARS = {0x09: "\t", 0x0D: "\r", 0x20: " ", 0x1B: "\x1b", 0x08: "\b"}
SYNTHETIC = 1 << 25  # a reserved lParam bit: tells the mod these keys come from this tool


def game_hwnd():
    hwnd = user32.FindWindowW("Gonzo", None)
    if not hwnd:
        raise SystemExit("game window not found")
    return hwnd


def post_modifier(hwnd, vk, down):
    lp = 1 | (user32.MapVirtualKeyW(vk, 0) << 16) | SYNTHETIC
    user32.PostMessageW(hwnd, WM_KEYDOWN if down else WM_KEYUP, vk, lp if down else lp | (1 << 30) | (1 << 31))


def press(hwnd, name):
    modifiers = []  # "shift+", "ctrl+" prefixes, in any order
    alt = False
    while True:
        if name.startswith("alt+"):
            alt = True
            name = name[4:]
            continue
        if name.startswith("shift+"):
            modifiers.append(VK["shift"])
            name = name[6:]
        elif name.startswith("ctrl+"):
            modifiers.append(VK["ctrl"])
            name = name[5:]
        else:
            break
    if name in VK:
        vk = VK[name]
    elif name.lower().startswith("f") and name[1:].isdigit():
        vk = 0x6F + int(name[1:])
    elif len(name) == 1:
        vk = ord(name.upper())
    else:
        raise SystemExit(f"unknown key {name}")
    scan = user32.MapVirtualKeyW(vk, 0)
    lp_down = 1 | (scan << 16) | SYNTHETIC | (ALT_CONTEXT if alt else 0)
    lp_up = lp_down | (1 << 30) | (1 << 31)
    down_msg, up_msg = (WM_SYSKEYDOWN, WM_SYSKEYUP) if alt else (WM_KEYDOWN, WM_KEYUP)
    # The keyboard state does not follow posted messages, so the mod tracks modifiers from the
    # messages of keys marked SYNTHETIC (and reads the physical keyboard for everything else).
    for m in modifiers:
        post_modifier(hwnd, m, True)
    user32.PostMessageW(hwnd, down_msg, vk, lp_down)
    # The game's TranslateMessage posts the WM_CHAR after the key-down is dispatched; a key-up
    # posted right away would come before it (a real keyboard sends char, then key-up), and the
    # game counts key-downs since the last key-up to tell a first press from a repeat.
    time.sleep(0.06)
    user32.PostMessageW(hwnd, up_msg, vk, lp_up)
    for m in reversed(modifiers):
        post_modifier(hwnd, m, False)


def main(argv):
    if not argv:
        print(__doc__)
        return 1
    delay = 0.6
    if "--delay" in argv:
        i = argv.index("--delay")
        delay = float(argv[i + 1])
        argv = argv[:i] + argv[i + 2:]
    hwnd = game_hwnd()
    for k in argv:
        press(hwnd, k)
        time.sleep(delay)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
