// Traces the buy/move tool (cMoveTool) while the virtual mouse moves and clicks over the lot:
// its mouse slots, the screen->world conversion 0x5e00e0 and the held object's placement.
// Build 1.10.7. Use with virtual_mouse.js and a --drive script.
const slots = { 0x56f2a0: 'slot5 create', 0x56dc90: 'slot7 move', 0x56ccc0: 'slot8 down', 0x56ec10: 'slot11 up',
                0x56d200: 'slot6 drop', 0x6338b0: 'commit', 0x633fd0: 'cursor place', 0x56ea30: 'slot22 begin' };
for (const k of Object.keys(slots)) {
    const name = slots[k];
    Interceptor.attach(va(parseInt(k)), {
        onEnter(args) {
            const t = this.context.ecx;
            this.self = t;
            let extra = '';
            if (name.startsWith('slot7') || name.startsWith('slot8') || name.startsWith('slot11')) {
                const pick = args[3];
                extra = ' a=' + args[0].toInt32() + ',' + args[1].toInt32() + ' obj=' + args[2].toInt32() +
                    ' pick=' + pick.readU32() + ' scr=' + pick.add(0xe).readS16() + ',' + pick.add(0xc).readS16() +
                    ' +97=' + t.add(0x97).readU8() + ' +81=' + t.add(0x81).readU8() + ' pickdump=' + Array.from(new Uint32Array(pick.readByteArray(0x20))).map(v => v.toString(16)).join(' ');
            }
            if (name === 'cursor place') extra = ' pos=' + args[0].readS32() + ',' + args[0].add(4).readS32() + ' lvl=' + args[1].toInt32() + ' ' + args[2] + ' ' + args[3] + ' ' + args[4];
            send(name + extra + ' <' + stack(this.context, 4));
        },
        onLeave() {
            if (name === 'cursor place') send('  placed=' + this.self.add(0x1b4).readS16() + ' err=' + va(0xa67078).readS32());
        }
    });
}
Interceptor.attach(va(0x5e00e0), {
    onEnter(args) { this.out = args[0]; this.inp = args[1].readU32(); this.flag = args[2].toInt32(); },
    onLeave() { send('s2w in=' + (this.inp & 0xffff) + ',' + (this.inp >>> 16) + ' flag=' + this.flag + ' -> ' + this.out.readS32() + ',' + this.out.add(4).readS32()); }
});
