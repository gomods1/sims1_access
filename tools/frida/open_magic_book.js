// Opens Makin' Magic's book window for the selected Sim, the way the debug cheat "magic_book" does
// (0x451142): operator new(0x4a8), CWinMagicBook ctor (the Sim's cXPerson, cookbook 0), show it
// modally through [0xa63570] (0x452320), then hand it to the window manager. Runs once, on the game
// thread, inside the game's window procedure; the run returns when the book is closed.
// python tools/frida_run.py frida/open_magic_book.js 4
const opNew = new NativeFunction(va(0x7a78be), 'pointer', ['uint32'], 'mscdecl');
const ctor = new NativeFunction(va(0x494390), 'pointer', ['pointer', 'pointer', 'uint32'], 'thiscall');
const reg = new NativeFunction(va(0x452320), 'void', ['pointer', 'pointer', 'uint32', 'uint32'], 'thiscall');
const add = new NativeFunction(va(0x6d9d10), 'void', ['pointer', 'pointer'], 'thiscall');
const selId = new NativeFunction(va(0x43bba0), 'int', [], 'mscdecl');
const person = new NativeFunction(va(0x43abd0), 'pointer', ['int'], 'mscdecl');
let done = false;
const h = Interceptor.attach(va(0x6f0e40), {
    onEnter() {
        if (done) return;
        done = true;
        try {
            const mem = opNew(0x4a8);
            const sim = person(selId());
            send('sim ' + sim);
            if (sim.isNull()) return;
            const book = ctor(mem, sim, 0);
            reg(va(0xa63570).readPointer(), book, 1, 0);
            add(va(0xa67044).readPointer().add(0x54).readPointer(), book);
            send('book ' + book);
        } catch (e) { send('error ' + e); }
    }
});
DISPOSERS.push(() => h.detach());
