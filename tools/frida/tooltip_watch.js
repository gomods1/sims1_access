// Prints every tooltip the game shows: hooks cDefaultTTWindow's set-text-and-render function
// (0x702a20, thiscall) and reads the window's stored std::string at this+0x128 after the call.
// Combined with speech.js, each new tooltip is also spoken.

let lastTip = null;
Interceptor.attach(va(0x702a20), {
    onEnter(args) { this.self = this.context.ecx; },
    onLeave() {
        try {
            const s = readStdString(this.self.add(0x128));
            if (s !== lastTip) {
                lastTip = s;
                send('TOOLTIP ' + JSON.stringify(s) + '  <- ' + where(this.returnAddress));
                if (s && typeof say === 'function') say(s); // speaks when combined with speech.js
            }
        } catch (e) { send('tooltip read failed: ' + e.message); }
    }
});
send('tooltip watch installed');
