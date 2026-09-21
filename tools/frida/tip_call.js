// Calls vtable slot 46 (ShowTooltip) of a widget on the main thread and reports the result.
Object.assign(EXPORTS, {
    showTip(w) {
        onMain(() => {
            const p = ptr(w);
            const fn = new NativeFunction(p.readPointer().add(46 * 4).readPointer(), 'pointer', ['pointer'], 'thiscall');
            const r = fn(p);
            send('slot46 -> ' + r);
        });
    }
});
