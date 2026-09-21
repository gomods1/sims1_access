// Shared helpers prepended to every script by frida_run.py.
const GAME = Process.getModuleByName('Sims.exe');

// Scripts add RPC functions with Object.assign(EXPORTS, {...}) so combined scripts don't clash.
// frida_run.py calls flush() (if any script defines it) before detaching.
const EXPORTS = {};
rpc.exports = EXPORTS;
// Cleanup callbacks run when the script unloads (Frida calls rpc.exports.dispose).
const DISPOSERS = [];
EXPORTS.dispose = () => { for (const d of DISPOSERS) { try { d(); } catch (e) { } } };
const PREFERRED_BASE = ptr('0x400000');

// Runtime address -> "Sims.exe!00xxxxxx" (preferred-base VA, matches Ghidra) or "module+off".
function where(addr) {
    const p = ptr(addr);
    const m = Process.findModuleByAddress(p);
    if (m === null) return p.toString();
    if (m.name === GAME.name) return 'Sims!' + p.sub(m.base).add(PREFERRED_BASE).toString().replace('0x', '');
    return m.name + '+' + p.sub(m.base).toString();
}

// Preferred-base VA (number or string) -> runtime pointer in this process.
function va(v) {
    return ptr(v).sub(PREFERRED_BASE).add(GAME.base);
}

function stack(ctx, depth) {
    return Thread.backtrace(ctx, Backtracer.ACCURATE).slice(0, depth || 6).map(where).join(' < ');
}

function exportOf(moduleName, name) {
    return Process.getModuleByName(moduleName).getExportByName(name);
}

// MSVC x86 std::string: {char buf[16] | char* ptr} @0, size @0x10, capacity @0x14 (STRUCTS.md).
// Text is Windows-1252; readAnsiString decodes with the system ANSI code page.
function readStdString(p) {
    const size = p.add(0x10).readU32();
    const cap = p.add(0x14).readU32();
    if (size > 4096 || cap < 15 || size > cap) return null;
    const data = cap > 15 ? p.readPointer() : p;
    return size === 0 ? '' : data.readAnsiString(size);
}
