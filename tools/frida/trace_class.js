// Hooks every virtual method of the classes listed in TRACE_CLASSES (injected by
// tools/trace_class.py from re/vtables.tsv), counts calls per method, and records up to a few
// samples of each method's arguments: ECX (`this`) and the first 5 stack slots, each shown as
// an int and, when it points at printable text, as that text.
// TRACE_TARGETS = [[va, label], ...] is prepended by trace_class.py.
const calls = new Map();
const samples = new Map();

function peekText(p) {
    try {
        const s = p.readCString(80);
        if (s !== null && s.length >= 2 && /^[\x20-\x7e -ÿ\r\n]+$/.test(s)) return JSON.stringify(s);
    } catch (e) { }
    return null;
}

function describe(v) {
    const t = peekText(v);
    return t !== null ? v + '=' + t : v.toString();
}

for (const [vaHex, label] of TRACE_TARGETS) {
    const target = va(vaHex);
    try {
        Interceptor.attach(target, {
            onEnter(args) {
                calls.set(label, (calls.get(label) || 0) + 1);
                const list = samples.get(label) || [];
                if (list.length < 3) {
                    const parts = ['ecx=' + describe(this.context.ecx)];
                    for (let i = 0; i < 5; i++) parts.push('a' + i + '=' + describe(args[i]));
                    list.push(parts.join(' ') + '  <- ' + where(this.returnAddress));
                    samples.set(label, list);
                }
            }
        });
    } catch (e) { send('cannot hook ' + label + ': ' + e.message); }
}
send('hooked ' + TRACE_TARGETS.length + ' methods');

Object.assign(EXPORTS, {
    flush() {
        const rows = [...calls.entries()].sort((a, b) => b[1] - a[1]);
        send('--- ' + rows.length + ' methods called ---');
        for (const [label, n] of rows) {
            send(n + '\t' + label);
            for (const s of samples.get(label) || []) send('\t\t' + s);
        }
    }
});
