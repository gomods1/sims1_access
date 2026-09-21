// Which GDI text APIs does the game call, with what strings, from where?
// Aggregates unique (api, text, caller chain) and reports counts on flush.
const seen = new Map();

function note(api, text, ctx) {
    const key = api + '\u0001' + text + '\u0001' + stack(ctx, 5);
    seen.set(key, (seen.get(key) || 0) + 1);
}

function hookA(dll, name, textArg, lenArg) {
    let fn;
    try { fn = exportOf(dll, name); } catch (e) { send('missing ' + dll + '!' + name); return; }
    Interceptor.attach(fn, {
        onEnter(args) {
            try {
                const len = lenArg === null ? -1 : args[lenArg].toInt32();
                const p = args[textArg];
                if (p.isNull()) return;
                const s = len < 0 ? p.readAnsiString() : p.readAnsiString(len);
                note(name, s, this.context);
            } catch (e) { note(name, '<unreadable>', this.context); }
        }
    });
}

function hookW(dll, name, textArg, lenArg) {
    let fn;
    try { fn = exportOf(dll, name); } catch (e) { send('missing ' + dll + '!' + name); return; }
    Interceptor.attach(fn, {
        onEnter(args) {
            try {
                const len = lenArg === null ? -1 : args[lenArg].toInt32();
                const p = args[textArg];
                if (p.isNull()) return;
                const s = len < 0 ? p.readUtf16String() : p.readUtf16String(len);
                note(name, s, this.context);
            } catch (e) { note(name, '<unreadable>', this.context); }
        }
    });
}

hookA('user32.dll', 'DrawTextA', 1, 2);
hookA('user32.dll', 'DrawTextExA', 1, 2);
hookW('user32.dll', 'DrawTextW', 1, 2);
hookA('gdi32.dll', 'TextOutA', 3, 4);
hookW('gdi32.dll', 'TextOutW', 3, 4);
hookA('gdi32.dll', 'ExtTextOutA', 5, 6);
hookW('gdi32.dll', 'ExtTextOutW', 5, 6);
hookA('gdi32.dll', 'GetTextExtentPoint32A', 1, 2);
hookW('gdi32.dll', 'GetTextExtentPoint32W', 1, 2);
send('text hooks installed');

Object.assign(EXPORTS, {
    flush() {
        const rows = [...seen.entries()].sort((a, b) => b[1] - a[1]);
        send('--- ' + rows.length + ' unique (api, text, caller) ---');
        for (const [k, n] of rows) {
            const [api, text, st] = k.split('\u0001');
            send(n + '\t' + api + '\t' + JSON.stringify(text) + '\t' + st);
        }
    }
});
