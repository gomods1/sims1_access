// Logs every SimAntics "Set Balloon/Headline" primitive (opcode 41, handler 0x61bc40, thiscall
// (frame*, operand*) on the object running the tree) with its decoded operand, the tree that ran
// it, and the headline record it leaves on the object (vector at +0x14c, 0x3c-byte entries).
// Read-only: nothing in the game is changed. Build 1.10.7 (b7c8765e).
//
// Operand (8 bytes, from the handler and the describer 0x65f8b0):
//   short 0: (index << 1) | 1 = on the stack object instead of self (index used by group 7 / 2)
//   byte 2: icon index in the group   byte 3: group (STR# 500 in Behavior.iff)
//   short 4: duration (-1 forever, 0 clear)   byte 6: balloon type (0 thought, 1 scream, 2 speak)
//   byte 7: 1 show when inactive, 2 crossed out, 4 anim reversed, 8 loops, 0x10 index + obj[+0x3a]
const GROUPS = ['old', 'balloon', 'conversation', 'motive', 'relationship', 'headline', 'social',
    'algorithmic', 'route failure', 'progress', 'conversation 2'];
const TYPES = ['thought', 'scream', 'speak', 'none'];

const neighborIdOf = new NativeFunction(va(0x43aac0), 'int', ['pointer'], 'mscdecl');
const neighborName = new NativeFunction(va(0x43af20), 'void', ['int', 'pointer'], 'mscdecl');

function objName(obj) {
    if (obj.isNull()) return 'null';
    try {
        const id = obj.add(0xf0).readS16();
        const type = obj.add(0x120).readPointer();
        const internal = type.isNull() ? '?' : type.add(0x18).readPointer().readCString();
        const nid = neighborIdOf(obj);
        if (nid >= 0) {
            const s = Memory.alloc(0x18);
            s.add(0x10).writeU32(0);
            s.add(0x14).writeU32(15);
            neighborName(nid, s);
            return `#${id} ${readStdString(s)} (n${nid})`;
        }
        return `#${id} ${internal}`;
    } catch (e) {
        return obj.toString();
    }
}

function objectById(module, id) {
    if (id <= 0) return ptr(0);
    const begin = module.add(0x18).readPointer();
    const end = module.add(0x1c).readPointer();
    if (id > end.sub(begin).toInt32() / 4) return ptr(0);
    return begin.add((id - 1) * 4).readPointer();
}

function hex(bytes) {
    return Array.from(new Uint8Array(bytes)).map(b => b.toString(16).padStart(2, '0')).join(' ');
}

Interceptor.attach(va(0x61bc40), {
    onEnter(args) {
        this.obj = this.context.ecx;
        const frame = args[0];
        const op = args[1];
        this.op = op;
        const raw = op.readByteArray(8);
        const b = new Uint8Array(raw);
        const short0 = op.readS16();
        const dur = op.add(4).readS16();
        const group = b[3], index = b[2], type = b[6], flags = b[7];
        const tree = frame.readU16(), instr = frame.add(2).readU16(), stackId = frame.add(4).readS16();
        const module = this.obj.add(0xdc).readPointer();
        const target = (b[0] & 1) ? objectById(module, stackId) : this.obj;
        this.target = target;
        let icon = '';
        if (group === 7) {
            if (index === 0) icon = ' icon of stack obj ' + objName(objectById(module, stackId));
            else if (index === 1) icon = ' icon of neighbor ' + stackId;
            else if (index === 2) icon = ' icon of local ' + (short0 >> 1);
        }
        this.line = `${objName(this.obj)} tree 0x${tree.toString(16)}:${instr} -> ${objName(target)}: ` +
            `${GROUPS[group] || group}/${index} ${TYPES[type] || type} dur ${dur} flags 0x${flags.toString(16)}` +
            `${icon} [${hex(raw)}] stackObj ${stackId}`;
    },
    onLeave() {
        let rec = '';
        try {
            const v = this.target.add(0x14c).readPointer();
            const e = this.target.add(0x150).readPointer();
            if (!v.isNull() && !v.equals(e)) rec = ' rec ' + hex(v.readByteArray(0x3c));
        } catch (err) { rec = ' rec ?'; }
        send(this.line + rec);
    },
});
send('balloon trace attached');
