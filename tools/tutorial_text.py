# memtool script: print the tutorial step panel (a non-modal cTSObjectDialog inside the lot view)
# and the tutorial arrow. python memtool.py -f tools/tutorial_text.py
main = u32(va(0xa7f484) + 0x2c) and u32(u32(va(0xa7f484)) + 0x2c)
def children(w):
    head = u32(w + 0x38); count = u32(w + 0x3c); out = []
    if not head or not count or count > 2000: return out
    n = u32(head)
    while n and n != head and len(out) < count:
        nx, _pv, val = u32s(n, 3); out.append(val); n = nx
    return out
for top in children(main):
    c = cls(top) or ''
    if c == 'cTSWinTutorialHighlight' and u32(top + 0xf4) & 1:
        print('arrow', u32s(top + 0xc, 4))
    if c != 'cDDDSimsView': continue
    for d in children(top):
        if (cls(d) or '') != 'cTSObjectDialog' or not (u32(d + 0xf4) & 1): continue
        for t in children(d):
            if (cls(t) or '') in ('cWinTextWrap', 'cTSWinText'):
                print('panel text:', repr(std(t + 0x50)))
