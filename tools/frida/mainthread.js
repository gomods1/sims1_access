// Runs queued jobs on the game's main thread: hooks PeekMessageA, which the main loop (0x712660)
// and every modal loop (cTSWinMgr 0x6d9f50 via 0x712560) call each iteration, and runs jobs only
// on the thread that owns the game window. Game functions must only be called from there.
const _mainJobs = [];
function onMain(fn) { _mainJobs.push(fn); }
const _gwtpid = new NativeFunction(exportOf('user32.dll', 'GetWindowThreadProcessId'), 'uint', ['pointer', 'pointer']);
const _findWnd = new NativeFunction(exportOf('user32.dll', 'FindWindowA'), 'pointer', ['pointer', 'pointer']);
const MAIN_TID = _gwtpid(_findWnd(Memory.allocAnsiString('Gonzo'), ptr(0)), ptr(0));
Interceptor.attach(exportOf('user32.dll', 'PeekMessageA'), {
    onEnter() {
        if (_mainJobs.length === 0 || Process.getCurrentThreadId() !== MAIN_TID) return;
        while (_mainJobs.length) {
            const job = _mainJobs.shift();
            try { job(); } catch (e) { send('main-thread job failed: ' + e.stack); }
        }
    }
});

const WM = () => va(0xa7f484).readPointer();
// cTSWinMgr::PostMsg(target, type, code, sender, extra): thiscall, queued (0x6dac50).
const _wmPost = new NativeFunction(va(0x6dac50), 'void', ['pointer', 'pointer', 'int', 'int', 'pointer', 'int'], 'thiscall');

// Same message a button sends its owner (+0x4c) on mouse-up (cTSWinBtn slot 65, 0x6ea2c0).
function clickButton(btn) {
    btn = ptr(btn);
    const owner = btn.add(0x4c).readPointer();
    _wmPost(WM(), owner, 3, 0x14, btn, 0);
}

Object.assign(EXPORTS, {
    clickBtn(btn) { onMain(() => { clickButton(btn); send("clicked " + btn); }); },
});

// Emulated mouse click delivered straight to a widget: the same two messages the window manager
// sends after hit testing (type 7 = left down -> slots 69+55, type 11 = left up -> slots 71+59),
// through the widget's own message handler (vtable slot 0). x, y = widget centre (absolute).
function pressWidget(w) {
    w = ptr(w);
    const l = w.add(0xc).readS32(), t = w.add(0x10).readS32(), r = w.add(0x14).readS32(), b = w.add(0x18).readS32();
    const x = (l + r) >> 1, y = (t + b) >> 1;
    const handler = new NativeFunction(w.readPointer().readPointer(), 'uint8', ['pointer', 'pointer'], 'thiscall');
    const msg = Memory.alloc(16);
    msg.writeU32(7); msg.add(4).writeS32(x); msg.add(8).writeS32(y); msg.add(12).writeU32(1);
    const a = handler(w, msg);
    msg.writeU32(11); msg.add(12).writeU32(0);
    const b2 = handler(w, msg);
    return [a, b2];
}
Object.assign(EXPORTS, {
    pressBtn(btn) { onMain(() => { send('pressed ' + btn + ' -> ' + JSON.stringify(pressWidget(btn))); }); },
});
