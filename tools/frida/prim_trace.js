// WARNING: detaching this script crashed the game once (fail-fast in frida-agent.dll after 15
// minutes on 0x615cb0, 2026-09-22): SimAntics throws C++ exceptions through the dispatcher.
// Only run it on a session you can lose.
// Logs selected SimAntics primitives as the object dispatcher (cXObject slot 0, 0x615cb0,
// thiscall (frame*, instruction*)) runs them: remove object instance (0x12), transfer funds (0x19),
// dialogs (0x24, 0x26, 0x27), create object instance (0x2a). Also logs the first time each tree
// runs on an object whose internal name looks like a burglar or the police. Read-only.
// Build 1.10.7 (b7c8765e). Instruction: u16 opcode, u8 true, u8 false, 8 operand bytes.
const WATCH = { 0x12: 'remove object', 0x19: 'transfer funds', 0x24: 'dialog private', 0x26: 'dialog global',
    0x27: 'dialog semiglobal', 0x2a: 'create object' };
const neighborIdOf2 = new NativeFunction(va(0x43aac0), 'int', ['pointer'], 'mscdecl');
const neighborName2 = new NativeFunction(va(0x43af20), 'void', ['int', 'pointer'], 'mscdecl');

function internalName(obj) {
    try {
        const type = obj.add(0x120).readPointer();
        return type.isNull() ? '?' : type.add(0x18).readPointer().readCString();
    } catch (e) { return '?'; }
}

function name2(obj) {
    if (obj.isNull()) return 'null';
    try {
        const id = obj.add(0xf0).readS16();
        const nid = neighborIdOf2(obj);
        if (nid >= 0) {
            const s = Memory.alloc(0x18);
            s.add(0x10).writeU32(0);
            s.add(0x14).writeU32(15);
            neighborName2(nid, s);
            return `#${id} ${readStdString(s)} (n${nid}, ${internalName(obj)})`;
        }
        return `#${id} ${internalName(obj)}`;
    } catch (e) { return obj.toString(); }
}

function byId(module, id) {
    if (id <= 0) return ptr(0);
    const begin = module.add(0x18).readPointer();
    const end = module.add(0x1c).readPointer();
    if (id > end.sub(begin).toInt32() / 4) return ptr(0);
    return begin.add((id - 1) * 4).readPointer();
}

const seenTrees = {};
Interceptor.attach(va(0x615cb0), {
    onEnter(args) {
        const obj = this.context.ecx;
        const frame = args[0], ins = args[1];
        const op = ins.readU16() & 0x7fff;
        const tree = frame.readU16();
        const internal = internalName(obj);
        const suspect = /burglar|police|cop|thief|robber/i.test(internal);
        if (!(op in WATCH) && !suspect) return;
        if (!(op in WATCH)) {
            const key = internal + ':' + tree;
            if (seenTrees[key]) return;
            seenTrees[key] = 1;
        }
        const module = obj.add(0xdc).readPointer();
        const stackId = frame.add(4).readS16();
        const b = new Uint8Array(ins.add(4).readByteArray(8));
        const bytes = Array.from(b).map(x => x.toString(16).padStart(2, '0')).join(' ');
        send(`${WATCH[op] || 'op 0x' + op.toString(16)} by ${name2(obj)} tree 0x${tree.toString(16)}:` +
            `${frame.add(2).readU16()} stack ${name2(byId(module, stackId))} [${bytes}]`);
    },
});
send('prim trace attached');
