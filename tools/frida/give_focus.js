// Hands the foreground to another window from inside the game. Needs frida/mainthread.js.
// The foreground process may always pass the foreground on; outside processes are refused.
// Use when the game has taken the user's focus: rpc.give_focus("0x204ca") from a drive script.
const _setForeground = new NativeFunction(exportOf('user32.dll', 'SetForegroundWindow'), 'int', ['pointer']);
const _getForeground = new NativeFunction(exportOf('user32.dll', 'GetForegroundWindow'), 'pointer', []);
Object.assign(EXPORTS, {
    giveFocus(hwnd) {
        onMain(() => send('SetForegroundWindow -> ' + _setForeground(ptr(hwnd)) + ', foreground now ' + _getForeground()));
    },
});
