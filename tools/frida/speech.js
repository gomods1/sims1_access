// Speech from inside Sims.exe: connects to SpeechHost's named pipe (mod/out/SpeechHost.exe,
// started by frida_run.py --speech) and writes "say 0 <utf8 text>\n". This is the same path the
// x86 mod DLL uses. say(text) is available to every script combined with this one.
const _createFileA = new NativeFunction(exportOf('kernel32.dll', 'CreateFileA'), 'pointer',
    ['pointer', 'uint', 'uint', 'pointer', 'uint', 'uint', 'pointer']);
const _writeFile = new NativeFunction(exportOf('kernel32.dll', 'WriteFile'), 'int',
    ['pointer', 'pointer', 'uint', 'pointer', 'pointer']);
const _closeHandle = new NativeFunction(exportOf('kernel32.dll', 'CloseHandle'), 'int', ['pointer']);

const PIPE_NAME = '\\\\.\\pipe\\SimsAccessSpeech-' + Process.id;
let speechPipe = null;

function openSpeechPipe() {
    const h = _createFileA(Memory.allocAnsiString(PIPE_NAME), 0x40000000 /* GENERIC_WRITE */, 0,
        ptr(0), 3 /* OPEN_EXISTING */, 0, ptr(0));
    if (h.equals(ptr('0xffffffff'))) return null;
    return h;
}

function pipeWrite(line) {
    if (speechPipe === null) speechPipe = openSpeechPipe();
    if (speechPipe === null) { send('speech: no SpeechHost pipe ' + PIPE_NAME); return; }
    const bytes = unescape(encodeURIComponent(line)); // JS string -> UTF-8 byte string
    const buf = Memory.alloc(bytes.length + 1);
    for (let i = 0; i < bytes.length; i++) buf.add(i).writeU8(bytes.charCodeAt(i));
    const written = Memory.alloc(4);
    if (!_writeFile(speechPipe, buf, bytes.length, written, ptr(0))) {
        _closeHandle(speechPipe);
        speechPipe = null;
        send('speech: pipe write failed');
    }
}

function say(text) {
    const clean = String(text).replace(/[\r\n]+/g, ' ').replace(/\s+/g, ' ').trim();
    if (clean.length === 0) return;
    send('SAY ' + clean);
    pipeWrite('say 0 ' + clean + '\n');
}

DISPOSERS.push(() => { if (speechPipe !== null) _closeHandle(speechPipe); speechPipe = null; });
