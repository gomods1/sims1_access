// Raw memory access for tools/memtool.py. Addresses are runtime addresses as strings/numbers;
// va(x) in memtool converts preferred-base VAs. Every read is fault-tolerant (returns null).
Object.assign(EXPORTS, {
    base() { return GAME.base.toString(); },
    readU32s(addr, n) {
        const out = [];
        try {
            const p = ptr(addr);
            for (let i = 0; i < n; i++) out.push(p.add(i * 4).readU32());
        } catch (e) { out.push(null); }
        return out;
    },
    readBytes(addr, n) {
        try { return Array.from(new Uint8Array(ptr(addr).readByteArray(n))); } catch (e) { return null; }
    },
    readAnsi(addr, n) {
        try { return ptr(addr).readAnsiString(n); } catch (e) { return null; }
    },
    readStd(addr) {
        try { return readStdString(ptr(addr)); } catch (e) { return null; }
    },
    readable(addr) {
        try { ptr(addr).readU8(); return true; } catch (e) { return false; }
    },
    writeU8(addr, v) { ptr(addr).writeU8(v); },
    writeU32(addr, v) { ptr(addr).writeU32(v); },
});

// Find every occurrence of an ANSI string in readable+writable memory (max 200 hits).
Object.assign(EXPORTS, {
    scanAnsi(text) {
        const bytes = [];
        for (let i = 0; i < text.length; i++) bytes.push(('0' + text.charCodeAt(i).toString(16)).slice(-2));
        const pattern = bytes.join(' ');
        const hits = [];
        for (const r of Process.enumerateRanges('rw-')) {
            try {
                for (const m of Memory.scanSync(r.base, r.size, pattern)) {
                    hits.push(m.address.toString());
                    if (hits.length >= 200) return hits;
                }
            } catch (e) { }
        }
        return hits;
    },
    scanU32(value) {
        const b = [value & 255, (value >>> 8) & 255, (value >>> 16) & 255, (value >>> 24) & 255];
        const pattern = b.map(x => ('0' + x.toString(16)).slice(-2)).join(' ');
        const hits = [];
        for (const r of Process.enumerateRanges('rw-')) {
            try {
                for (const m of Memory.scanSync(r.base, r.size, pattern)) {
                    if (m.address.and(3).toInt32() !== 0) continue;
                    hits.push(m.address.toString());
                    if (hits.length >= 500) return hits;
                }
            } catch (e) { }
        }
        return hits;
    },
});
