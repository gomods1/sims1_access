// Places the held buy-mode object on a chosen tile without the mouse (use with mainthread.js):
// the screen->world conversion 0x5e00e0 is answered with the tile's view coordinates, then the
// move tool gets the mouse-down (slot 8) and mouse-up (slot 11) a click would give it.
// Build 1.10.7. rpc: place(tileX, tileY, level)
const WORLD = () => va(0xa675b4).readPointer();
let target = null;  // {vx, vy} view coordinates in 1/16 tile
const s2wOriginal = new NativeFunction(va(0x5e00e0), 'pointer', ['pointer', 'pointer', 'int'], 'mscdecl');
Interceptor.replace(va(0x5e00e0), new NativeCallback((out, screen, flag) => {
    const r = s2wOriginal(out, screen, flag);
    if (target !== null) {
        target.calls = (target.calls || 0) + 1;
        target.orig = out.readS32() + ',' + out.add(4).readS32();
        out.writeS32(target.vx);
        out.add(4).writeS32(target.vy);
    }
    return r;
}, 'pointer', ['pointer', 'pointer', 'int'], 'mscdecl'));
function gridToView(gx, gy) {
    const r = (-WORLD().add(0x84).readS32()) & 3, n = 0x3ff;
    switch (r) {
    case 1: return [n - gy, gx];
    case 2: return [n - gx, n - gy];
    case 3: return [gy, n - gx];
    default: return [gx, gy];
    }
}
function vcall(obj, slot, ret, args, values) {
    const fn = new NativeFunction(obj.readPointer().add(slot * 4).readPointer(), ret, ['pointer'].concat(args), 'thiscall');
    return fn(obj, ...values);
}
Object.assign(EXPORTS, {
    place(tx, ty) {
        onMain(() => {
            const tool = va(0xa67108).readPointer();
            const [vx, vy] = gridToView(tx * 16 + 8, ty * 16 + 8);
            target = { vx: vx, vy: vy };
            const pick = Memory.alloc(0x40);
            pick.writeU32(12);
            pick.add(4).writePointer(va(0xa7f484).readPointer().add(0x24).readPointer());
            pick.add(0xc).writeS16(300);   // screen y (game coordinates)
            pick.add(0xe).writeS16(512);   // screen x
            const cursor = tool.add(0x3c).readPointer();
            send('before: cursor ' + cursor + ' placed ' + (cursor.isNull() ? '-' : cursor.add(0x1b4).readS16()));
            send('+7b ' + tool.add(0x7b).readU8() + ' +81 ' + tool.add(0x81).readU8());
            tool.add(0x7b).writeU8(0);
            vcall(tool, 7, 'void', ['int', 'int', 'int', 'pointer'], [vx, vy, 0, pick]);
            send('s2w calls ' + target.calls + ' orig ' + target.orig + ' target ' + vx + ',' + vy);
            send('after move: placed ' + cursor.add(0x1b4).readS16() + ' err ' + va(0xa67078).readS32() +
                 ' obj at ' + cursor.add(0xf4).readS32() + ',' + cursor.add(0xf8).readS32());
            vcall(tool, 8, 'void', ['int', 'int', 'int', 'pointer'], [vx, vy, 0, pick]);
            vcall(tool, 11, 'void', ['int', 'int', 'int', 'pointer'], [vx, vy, 0, pick]);
            target = null;
            send('after up: cursor ' + tool.add(0x3c).readPointer() + ' err ' + va(0xa67078).readS32());
        });
    }
});
