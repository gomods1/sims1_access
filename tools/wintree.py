# memtool script: print the live cTSWin tree from the window manager's root window.
#   python memtool.py -f wintree.py            (whole tree)
# Offsets: STRUCTS.md (cTSWin). Visible = flag 0x1 at +0xf4; buttons show enabled (+0x1b8).
import os

MAX_DEPTH = int(os.environ.get("WINTREE_DEPTH", "12"))
SHOW_HIDDEN = os.environ.get("WINTREE_HIDDEN", "1") == "1"


def children(w):
    head = u32(w + 0x38)
    count = u32(w + 0x3c)
    out = []
    if not head or not count or count > 2000:
        return out
    n = u32(head)
    while n and n != head and len(out) < count:
        nx, _pv, val = u32s(n, 3)
        out.append(val)
        n = nx
    return out


def describe(w):
    c = cls(w) or "?"
    l, t, r, b = u32s(w + 0x0c, 4)
    flags = u32(w + 0xf4)
    tip = std(w + 0xbc) or ""
    extra = ""
    if "Btn" in c or c in ("cWinLotBtn", "cCustButton", "ProductButton", "cWinMotive", "cWinGift",
                           "cWinInterest", "cWinPersonality", "cWinRelationship", "cTSSystemButton"):
        en, pressed = M.rpc.read_bytes(w + 0x1b8, 2)
        extra = f" en={en} st={u32(w+0x138)} type={u32(w+0x148)} chk={u32(w+0x130)}"
    return f"{c} [{l},{t},{r},{b}] f={flags:#x}{extra} {tip!r}"


def walk(w, depth, parent_visible):
    vis = parent_visible and bool(u32(w + 0xf4) & 1)
    if vis or SHOW_HIDDEN:
        print("  " * depth + ("" if vis else "(hidden) ") + f"{w:#x} " + describe(w))
    if depth < MAX_DEPTH:
        for c in children(w):
            walk(c, depth + 1, vis)


wm = G(0xa7f484)
print("wm +0x24", cls(u32(wm + 0x24)), "+0x28", cls(u32(wm + 0x28)), "+0x17c", cls(u32(wm + 0x17c)),
      "+0x210", cls(u32(wm + 0x210)))
walk(u32(wm + 0x2c), 0, True)
