// Virtual mouse: makes GetCursorPos return a chosen point for calls made from Sims.exe, and
// posts mouse messages straight to the game window. The user's real cursor is never moved.
const getCursorPos = exportOf('user32.dll', 'GetCursorPos');
const postMessageA = new NativeFunction(exportOf('user32.dll', 'PostMessageA'), 'int', ['pointer', 'uint', 'uint', 'pointer']);
const findWindowA2 = new NativeFunction(exportOf('user32.dll', 'FindWindowA'), 'pointer', ['pointer', 'pointer']);
const gameHwnd = findWindowA2(Memory.allocAnsiString('Gonzo'), ptr(0));

let fake = null; // {x, y} in screen coordinates (the game window is at 0,0)

// The game drops all input while its "app active" byte is 0 (set from WM_ACTIVATEAPP in the
// message handler at 0x6f1699; checked by the input dispatcher 0x6dae40). The user keeps other
// windows in front during sessions, so force it on while driving.
const APP_ACTIVE = va(0xa52074);
function forceActive() { APP_ACTIVE.writeU8(1); }

Interceptor.attach(getCursorPos, {
    onEnter(args) { this.pt = args[0]; this.fromGame = GAME.base.compare(this.returnAddress) <= 0 && this.returnAddress.compare(GAME.base.add(GAME.size)) < 0; },
    onLeave(retval) {
        if (fake !== null && this.fromGame) {
            this.pt.writeS32(fake.x);
            this.pt.add(4).writeS32(fake.y);
        }
    }
});

const screenToClient = new NativeFunction(exportOf('user32.dll', 'ScreenToClient'), 'int', ['pointer', 'pointer']);

// Mouse messages carry client coordinates; the fake cursor is in screen coordinates.
// The game moves the real cursor with SetCursorPos (pie menus near an edge or opening a
// sub-level). Never let it touch the user's cursor: swallow the call and move the fake one.
Interceptor.replace(exportOf('user32.dll', 'SetCursorPos'), new NativeCallback((x, y) => {
    fake = { x: x, y: y };
    send('blocked SetCursorPos(' + x + ', ' + y + ')');
    return 1;
}, 'int', ['int', 'int']));

function lparam(x, y) {
    const pt = Memory.alloc(8);
    pt.writeS32(x);
    pt.add(4).writeS32(y);
    screenToClient(gameHwnd, pt);
    const cx = pt.readS32(), cy = pt.add(4).readS32();
    return ptr(((cy & 0xffff) << 16) | (cx & 0xffff));
}

Object.assign(EXPORTS, {
    setPos(x, y) {
        forceActive();
        fake = { x: x, y: y };
        postMessageA(gameHwnd, 0x200, 0, lparam(x, y)); // WM_MOUSEMOVE
    },
    click(x, y) {
        forceActive();
        fake = { x: x, y: y };
        postMessageA(gameHwnd, 0x200, 0, lparam(x, y));
        postMessageA(gameHwnd, 0x201, 1, lparam(x, y)); // WM_LBUTTONDOWN, MK_LBUTTON
        postMessageA(gameHwnd, 0x202, 0, lparam(x, y)); // WM_LBUTTONUP
    },
    release() { fake = null; }
});
send('virtual mouse ready, hwnd ' + gameHwnd);
