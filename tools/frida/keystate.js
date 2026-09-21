const getKeyState = new NativeFunction(exportOf('user32.dll', 'GetKeyState'), 'int16', ['int']);
const getAsync = new NativeFunction(exportOf('user32.dll', 'GetAsyncKeyState'), 'int16', ['int']);
onMain(() => {
    const vks = {SHIFT: 0x10, CTRL: 0x11, ALT: 0x12, LBUTTON: 1, LSHIFT: 0xa0, LMENU: 0xa4, RMENU: 0xa5};
    const out = [];
    for (const k in vks) out.push(k + ' state=' + getKeyState(vks[k]) + ' async=' + getAsync(vks[k]));
    send(out.join(' | '));
});
