// The mod's own settings, kept in SimsAccess\SimsAccess.ini ([mod] section) so a player can also
// edit them in a text editor. Values the mod changes by key (the narrator's verbosity) are saved
// at once.
#pragma once

namespace util {

void InitSettings(const wchar_t* modDir);
int GetSetting(const char* key, int fallback);
void SetSetting(const char* key, int value);

}  // namespace util
