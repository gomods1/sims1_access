// Speech through SpeechHost.exe (x64, owns Prism) over a named pipe. All calls are cheap and
// non-blocking: lines are queued and written by a background thread, so a slow or missing host
// never stalls the game.
#pragma once

#include <string>

namespace speech {

void Start(const wchar_t* modDir);  // modDir holds SpeechHost.exe; starts it if not running
void Stop();                        // closes the pipe; the host stays alive for a reloaded core
void Say(const std::string& utf8, bool interrupt = false);
void Silence();                     // stop current speech
// A sound cue (SpeechHost plays SimsAccess\sounds\<name>.wav, mixed beside the screen reader):
// pan -1 left .. 1 right, pitch as a speed ratio (0.5 .. 2), volume 0 .. 1, after delayMs.
void Cue(const char* name, float pan = 0, float pitch = 1, float volume = 1, int delayMs = 0);

}  // namespace speech
