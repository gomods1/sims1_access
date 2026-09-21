// Counts polled key/button checks (cTSKeyboard wrappers around GetAsyncKeyState) by key and caller.
const polls = new Map();
for (const fn of [0x71e6a0, 0x71e6c0]) {
    Interceptor.attach(va(fn), {
        onEnter(args) {
            const vk = fn === 0x71e6a0 ? args[0].toInt32() : this.context.esp.add(4).readU32();
            const key = where(fn) + ' vk=0x' + vk.toString(16) + ' <- ' + where(this.returnAddress);
            polls.set(key, (polls.get(key) || 0) + 1);
        }
    });
}
const gaks = exportOf('user32.dll', 'GetAsyncKeyState');
Interceptor.attach(gaks, {
    onEnter(args) {
        const r = this.returnAddress;
        if (GAME.base.compare(r) > 0 || r.compare(GAME.base.add(GAME.size)) >= 0) return;
        const key = 'GetAsyncKeyState vk=0x' + args[0].toInt32().toString(16) + ' <- ' + where(r);
        polls.set(key, (polls.get(key) || 0) + 1);
    }
});
Object.assign(EXPORTS, { flush() { for (const [k, n] of polls) send(n + '\t' + k); } });
