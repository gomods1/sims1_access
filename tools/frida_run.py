"""Attach a Frida script to the running Sims.exe for a bounded time and print its messages.

  python frida_run.py <script.js[,more.js]> [seconds=20] [--drive <python-file>] [--speech|--speech-mute]

Scripts send() plain strings or objects; both are printed one per line. Sims.exe addresses in
scripts should be reported as preferred-base VAs (0x400000 + offset) so they match Ghidra and
CLAUDE.md; tools/frida/common.js has helpers for that.
--drive runs a Python file while the script is attached, with `script` (the Frida script) and
`rpc` (its exports_sync) in its globals, to drive the game; see tools/drive_*.py.
--speech starts mod/out/SpeechHost.exe for the game PID before attaching (combine with
frida/speech.js so scripts can say()); --speech-mute does the same but SpeechHost only logs the
lines (mod/out/SpeechHost.log) without speaking. Speech talks through the user's screen reader:
ask before using --speech.
"""

import ctypes
import os
import runpy
import subprocess
import sys
import threading
import time

import frida

HERE = os.path.dirname(os.path.abspath(__file__))
SPEECH_HOST = os.path.join(os.path.dirname(HERE), "mod", "out", "SpeechHost.exe")


def start_speech_host(pid: int, mute: bool) -> subprocess.Popen:
    # One host per game PID owns the pipe; replace any left over from an earlier run.
    subprocess.run(["taskkill", "/IM", "SpeechHost.exe", "/F"], capture_output=True)
    args = [SPEECH_HOST, "--pid", str(pid)] + (["--mute"] if mute else [])
    host = subprocess.Popen(args)
    pipe = rf"\\.\pipe\SimsAccessSpeech-{pid}"
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    for _ in range(50):
        # WaitNamedPipe probes without connecting (opening the pipe would use up the instance).
        if kernel32.WaitNamedPipeW(pipe, 1) or ctypes.get_last_error() == 121:  # ERROR_SEM_TIMEOUT
            return host
        time.sleep(0.1)
    raise SystemExit("SpeechHost did not create its pipe; see mod/out/SpeechHost.log")


def main(argv: list[str]) -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    if not argv:
        print(__doc__)
        return 1
    script_path = argv[0]
    seconds = float(argv[1]) if len(argv) > 1 and not argv[1].startswith("--") else 20.0
    drive = argv[argv.index("--drive") + 1] if "--drive" in argv else None

    common = open(os.path.join(HERE, "frida", "common.js"), encoding="utf-8").read()
    # Several scripts can be combined with commas (a.js,b.js); they share one scope and add their
    # RPC functions to EXPORTS (defined in common.js).
    source = common + "\n" + "\n".join(
        open(p, encoding="utf-8").read() for p in script_path.split(","))

    session = frida.attach("Sims.exe")
    host = None
    if "--speech" in argv or "--speech-mute" in argv:
        host = start_speech_host(session.pid, "--speech-mute" in argv)
    script = session.create_script(source)
    lock = threading.Lock()

    def on_message(msg, _data):
        with lock:
            if msg["type"] == "send":
                print(msg["payload"], flush=True)
            else:
                print("[frida]", msg.get("description", msg), msg.get("stack", ""), flush=True)

    script.on("message", on_message)
    script.load()
    try:
        if drive:
            runpy.run_path(drive, init_globals={"script": script, "rpc": script.exports_sync})
        remaining = seconds
        while remaining > 0:
            time.sleep(min(1.0, remaining))
            remaining -= 1.0
        try:
            script.exports_sync.flush()
        except Exception:
            pass
    finally:
        script.unload()
        session.detach()
        if host is not None:
            time.sleep(1.0)  # let queued lines reach the screen reader
            host.kill()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
