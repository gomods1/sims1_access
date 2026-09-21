// Logs the picker's "execute chosen interaction" call and the opener's return.
Interceptor.attach(va(0x5315d0), { onEnter(args) { send('push interaction obj=' + (args[0].toInt32() & 0xffff) + ' a1=' + args[1] + ' a2=' + args[2] + ' <- ' + where(this.returnAddress)); } });
Interceptor.attach(va(0x61fe70), { onLeave(r) { send('menu runner returned ' + r.toInt32()); } });
