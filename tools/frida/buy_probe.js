// Buy mode experiments (use with mainthread.js): the move tool's held "cursor object" and the
// engine's placement check. Build 1.10.7 (b7c8765e). Run while a catalog product is chosen.
//   python tools/frida_run.py frida/mainthread.js,frida/buy_probe.js 8
const tool = () => va(0xa67108).readPointer();
const errCode = () => va(0xa67078).readS32();
function vcall(obj, slot, ret, args, values) {
    const fn = new NativeFunction(obj.readPointer().add(slot * 4).readPointer(), ret, ['pointer'].concat(args), 'thiscall');
    return fn(obj, ...values);
}
function simPos() {
    const mod = va(0xa67044).readPointer().readPointer().add(0x14).readPointer();
    const people = mod.add(0x80).readPointer();
    const p = people.readPointer();
    return [p.add(0xf4).readS32(), p.add(0xf8).readS32(), p.add(0xfc).readU32()];
}
onMain(() => {
    const t = tool();
    send('tool mode ' + t.add(0x34).readS32() + ' type ' + t.add(0x44).readPointer() + ' cursor ' + t.add(0x3c).readPointer());
    if (t.add(0x3c).readPointer().isNull()) {
        vcall(t, 5, 'void', [], []);  // 0x56f2a0: creates the cursor object for the chosen product
        send('after slot 5: cursor ' + t.add(0x3c).readPointer());
    }
    const c = t.add(0x3c).readPointer();
    if (c.isNull()) return;
    const obj = c.add(0x1b0).readPointer();
    send('cursor obj ' + obj + ' dir ' + (obj.isNull() ? '-' : obj.add(0x4c).readS16()) + ' placed ' + c.add(0x1b4).readS16());
    const [sx, sy, pl] = simPos();
    send('sim at ' + (sx >> 4) + ',' + (sy >> 4) + ' placement ' + pl.toString(16));
    const pos = Memory.alloc(8);
    const lines = [];
    for (let dy = -3; dy <= 3; dy++) {
        for (let dx = -3; dx <= 3; dx++) {
            const x = ((sx >> 4) + dx) * 16 + 8, y = ((sy >> 4) + dy) * 16 + 8;
            pos.writeS32(x); pos.add(4).writeS32(y);
            va(0xa67078).writeS32(0);
            const ok = vcall(c, 21, 'uint8', ['pointer', 'int', 'int', 'int'], [pos, 1, 0, 0]);  // CanPlace
            lines.push((dx) + ',' + (dy) + ':' + ok + '/' + errCode());
        }
    }
    send(lines.join(' '));
});
