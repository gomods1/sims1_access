// Prints cObjPickerTool slot 30 (open interaction menu) arguments and the pick-info struct.
Interceptor.attach(va(0x621960), {
    onEnter(args) {
        const info = args[3];
        const bytes = Array.from(new Uint8Array(info.readByteArray(0x20))).map(b => ('0' + b.toString(16)).slice(-2)).join(' ');
        send('slot30 x=' + args[0].toInt32() + ' y=' + args[1].toInt32() + ' obj=' + (args[2].toInt32() & 0xffff) +
             ' info=' + info + ' [' + bytes + '] tool=' + this.context.ecx + ' cur=' + va(0xa67108).readPointer());
    }
});
