// Logs every UI message delivered to one widget through its own message handler (vtable slot 0;
// cTSWin's is 0x6dd7a0, some classes override it). Call rpc.watch(addr) first.
// Message layout: {type, a, b, c}.
let watchTarget = null;
const hooked = new Set();
Object.assign(EXPORTS, {
    watch(a) {
        watchTarget = ptr(a);
        const handler = watchTarget.readPointer().readPointer();
        if (!hooked.has(handler.toString())) {
            hooked.add(handler.toString());
            Interceptor.attach(handler, {
                onEnter(args) {
                    if (watchTarget === null || !this.context.ecx.equals(watchTarget)) return;
                    const m = args[0];
                    send('MSG type=' + m.readU32() + ' a=' + m.add(4).readS32() + ' b=' + m.add(8).readS32() +
                         ' c=' + m.add(12).readS32() + ' <- ' + where(this.returnAddress));
                }
            });
        }
        send('watching ' + a + ' handler ' + where(handler));
    }
});
