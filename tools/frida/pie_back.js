// Ends the open top-level interaction menu with result -1 (back/close) on the main thread.
onMain(() => {
    const pie = va(0xa7f484).readPointer().add(0x24).readPointer();
    const fn = new NativeFunction(pie.readPointer().add(147 * 4).readPointer(), 'void', ['pointer', 'int'], 'thiscall');
    fn(pie, -1);
    send('sent -1 to ' + pie);
});
