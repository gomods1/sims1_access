// How does the game read input? Counts calls to the polling APIs (with callers) and logs the
// messages reaching the main window procedure. Idle run = what is polled every frame.
const counts = new Map();
function bump(key) { counts.set(key, (counts.get(key) || 0) + 1); }

function hookCount(dll, name, describe) {
    Interceptor.attach(exportOf(dll, name), {
        onEnter(args) { bump(name + ' ' + (describe ? describe(args) : '') + ' <- ' + where(this.returnAddress)); }
    });
}

hookCount('user32.dll', 'GetCursorPos');
hookCount('user32.dll', 'ScreenToClient');
hookCount('user32.dll', 'GetAsyncKeyState', a => 'vk=' + a[0].toInt32().toString(16));
hookCount('user32.dll', 'GetKeyState', a => 'vk=' + a[0].toInt32().toString(16));
hookCount('user32.dll', 'PeekMessageA');
hookCount('user32.dll', 'GetFocus');
hookCount('user32.dll', 'GetForegroundWindow');

// Main window: class "Gonzo".
const findWindowA = new NativeFunction(exportOf('user32.dll', 'FindWindowA'), 'pointer', ['pointer', 'pointer']);
const getWindowLongA = new NativeFunction(exportOf('user32.dll', 'GetWindowLongA'), 'int', ['pointer', 'int']);
const hwnd = findWindowA(Memory.allocAnsiString('Gonzo'), ptr(0));
const wndproc = ptr(getWindowLongA(hwnd, -4) >>> 0);
send('hwnd ' + hwnd + ' wndproc ' + where(wndproc));

const msgs = new Map();
Interceptor.attach(wndproc, {
    onEnter(args) {
        const m = args[1].toInt32() >>> 0;
        msgs.set(m, (msgs.get(m) || 0) + 1);
    }
});

Object.assign(EXPORTS, {
    flush() {
        send('--- polled APIs (count, api, caller) ---');
        for (const [k, n] of [...counts.entries()].sort((a, b) => b[1] - a[1]).slice(0, 60)) send(n + '\t' + k);
        send('--- window messages (msg id hex: count) ---');
        send([...msgs.entries()].sort((a, b) => a[0] - b[0]).map(([m, n]) => m.toString(16) + ':' + n).join(' '));
    }
});
