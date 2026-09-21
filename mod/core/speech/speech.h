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

}  // namespace speech
