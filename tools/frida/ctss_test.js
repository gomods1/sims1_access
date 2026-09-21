onMain(() => {
    const root = va(0xa67044).readPointer().readPointer();
    const world = root.add(0x14).readPointer();
    const objs = world.add(0x18).readPointer();
    for (const oid of [171, 6, 78]) {
        const o = objs.add((oid - 1) * 4).readPointer();
        const type = o.add(0x120).readPointer();
        const resFile = type.readPointer();
        const objd = o.add(0x11c).readPointer();
        const ctss = objd.add(41 * 2).readU16();
        const set = Memory.alloc(0x100);
        const ctor = new NativeFunction(va(0x58c2f0), 'void', ['pointer'], 'thiscall');
        const src = new NativeFunction(va(0x58ef00), 'void', ['pointer', 'pointer', 'int16', 'uint32', 'int8', 'int8'], 'thiscall');
        const load = new NativeFunction(va(0x58d4c0), 'int', ['pointer', 'int8', 'int8'], 'thiscall');
        const get = new NativeFunction(va(0x58cd60), 'pointer', ['pointer', 'int', 'int8'], 'thiscall');
        const dtor = new NativeFunction(va(0x58c4b0), 'void', ['pointer'], 'thiscall');
        ctor(set);
        src(set, resFile, ctss, 0x43545353, 0, -1);
        const rc = load(set, 1, 0);
        let names = [];
        for (let i = 1; i <= 2; i++) { const p = get(set, i, -1); names.push(p.isNull() ? null : p.readAnsiString()); }
        const lang = set.add(0x5d).readS8();
        send('obj ' + oid + ' ctss ' + ctss + ' load rc=' + rc + ' lang=' + lang + ' names=' + JSON.stringify(names) + ' tbl=' + set.add(0x6c).readPointer());
        dtor(set);
    }
});
