// The game's settings in the registry (HKCU\Software\Electronic Arts\The Sims 25): language,
// current neighborhood... The game keeps them current while it runs.
#pragma once

#include <string>

namespace util {

// A string value; empty when missing.
std::string ReadGameSetting(const char* name);

}  // namespace util
