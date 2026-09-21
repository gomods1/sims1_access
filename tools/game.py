"""Launch, inspect and stop The Sims Legacy Collection during RE sessions.

  python game.py launch [args...]   start through Steam (steam://run/3314060//args/) and print the PID
  python game.py pid                print the running Sims.exe PID
  python game.py windows            list the game's top-level windows (hwnd, class, title, rect, style)
  python game.py shot [name]        screenshot the game's main window to shots/<name>.png
  python game.py kill               terminate Sims.exe (by PID)
"""

import ctypes
import ctypes.wintypes as wt
import os
import subprocess
import sys
import time

APP_ID = 3314060
STEAM_EXE = r"S:\steam\steam.exe"
EXE_NAME = "Sims.exe"
SHOT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "shots")

user32 = ctypes.WinDLL("user32", use_last_error=True)
user32.SetProcessDPIAware()


def find_pid() -> int | None:
    out = subprocess.run(
        ["tasklist", "/FI", f"IMAGENAME eq {EXE_NAME}", "/FO", "CSV", "/NH"],
        capture_output=True, text=True,
    ).stdout
    for line in out.splitlines():
        parts = [p.strip('"') for p in line.split('","')]
        if parts and parts[0].lower() == EXE_NAME.lower():
            return int(parts[1])
    return None


def windows_of(pid: int) -> list[dict]:
    found = []
    proto = ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)

    def cb(hwnd, _):
        wpid = wt.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(wpid))
        if wpid.value == pid:
            title = ctypes.create_unicode_buffer(256)
            cls = ctypes.create_unicode_buffer(256)
            user32.GetWindowTextW(hwnd, title, 256)
            user32.GetClassNameW(hwnd, cls, 256)
            rect = wt.RECT()
            user32.GetWindowRect(hwnd, ctypes.byref(rect))
            client = wt.RECT()
            user32.GetClientRect(hwnd, ctypes.byref(client))
            origin = wt.POINT(0, 0)
            user32.ClientToScreen(hwnd, ctypes.byref(origin))
            found.append({
                "hwnd": hwnd, "class": cls.value, "title": title.value,
                "visible": bool(user32.IsWindowVisible(hwnd)),
                "rect": (rect.left, rect.top, rect.right, rect.bottom),
                "client": (origin.x, origin.y, client.right, client.bottom),
                "style": user32.GetWindowLongW(hwnd, -16) & 0xFFFFFFFF,
            })
        return True

    user32.EnumWindows(proto(cb), 0)
    return found


def main_window(pid: int) -> dict | None:
    wins = [w for w in windows_of(pid) if w["visible"] and w["client"][2] > 0]
    return max(wins, key=lambda w: w["client"][2] * w["client"][3], default=None)


def launch(args: list[str]) -> int:
    if find_pid():
        print("already running, pid", find_pid())
        return find_pid()
    url = f"steam://run/{APP_ID}//{' '.join(args)}/" if args else f"steam://rungameid/{APP_ID}"
    subprocess.Popen([STEAM_EXE, url])
    deadline = time.time() + 180
    while time.time() < deadline:
        pid = find_pid()
        if pid:
            print("pid", pid)
            return pid
        time.sleep(1)
    raise SystemExit("Sims.exe did not start within 180 s")


def capture_window(hwnd: int, cw: int, ch: int) -> bytes:
    """PrintWindow(PW_RENDERFULLCONTENT) into a DIB: works while the game is behind other windows.

    Never grab the screen instead: the user works on this PC during sessions, and the screen shows
    their windows, not the game.
    """
    gdi32 = ctypes.windll.gdi32
    hdc_win = user32.GetDC(hwnd)
    hdc = gdi32.CreateCompatibleDC(hdc_win)
    bmp = gdi32.CreateCompatibleBitmap(hdc_win, cw, ch)
    gdi32.SelectObject(hdc, bmp)
    ok = user32.PrintWindow(hwnd, hdc, 3)  # PW_CLIENTONLY | PW_RENDERFULLCONTENT
    bmi = (ctypes.c_byte * 40)()
    ctypes.memmove(bmi, (ctypes.c_int32 * 4)(40, cw, -ch, 0x200001), 16)  # top-down, 1 plane, 32 bpp
    buf = ctypes.create_string_buffer(cw * ch * 4)
    gdi32.GetDIBits(hdc, bmp, 0, ch, buf, bmi, 0)
    gdi32.DeleteObject(bmp)
    gdi32.DeleteDC(hdc)
    user32.ReleaseDC(hwnd, hdc_win)
    if not ok:
        raise SystemExit("PrintWindow failed")
    return buf.raw


def shot(name: str) -> str:
    pid = find_pid()
    if not pid:
        raise SystemExit("not running")
    w = main_window(pid)
    if not w:
        raise SystemExit("no visible window")
    _, _, cw, ch = w["client"]
    bgra = capture_window(w["hwnd"], cw, ch)
    os.makedirs(SHOT_DIR, exist_ok=True)
    path = os.path.join(SHOT_DIR, f"{name}.png")
    from PIL import Image
    Image.frombuffer("RGBA", (cw, ch), bgra, "raw", "BGRA", 0, 1).convert("RGB").save(path)
    print(path, f"{cw}x{ch}")
    return path


def main(argv: list[str]) -> int:
    cmd = argv[0] if argv else ""
    if cmd == "launch":
        launch(argv[1:])
    elif cmd == "pid":
        print(find_pid())
    elif cmd == "windows":
        pid = find_pid()
        for w in windows_of(pid) if pid else []:
            print(f"{w['hwnd']:#x} vis={w['visible']} style={w['style']:#010x} rect={w['rect']} "
                  f"client={w['client']} class={w['class']!r} title={w['title']!r}")
    elif cmd == "shot":
        shot(argv[1] if len(argv) > 1 else time.strftime("%H%M%S"))
    elif cmd == "kill":
        pid = find_pid()
        if pid:
            subprocess.run(["taskkill", "/PID", str(pid), "/F"], capture_output=True)
            print("killed", pid)
    else:
        print(__doc__)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
