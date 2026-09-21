// Logs text drawn through cTSFont slot 25 (0x723a70, thiscall (const char* text, int len, ...))
// and slot 24 (0x723ad0, one wrapped line), with the caller. Used to see what popups draw.
function hookText(vaddr, tag) {
    Interceptor.attach(va(vaddr), {
        onEnter(args) {
            try {
                const len = args[1].toInt32();
                if (len <= 0 || len > 2000) return;
                const s = args[0].readAnsiString(len);
                send(tag + ' ' + JSON.stringify(s) + ' <- ' + where(this.returnAddress));
            } catch (e) { }
        }
    });
}
hookText(0x723a70, 'TEXT25');
hookText(0x723ad0, 'LINE24');
