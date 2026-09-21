#include "game_settings.h"

#include <windows.h>

namespace util {

std::string ReadGameSetting(const char* name) {
    char value[260] = {};
    DWORD size = sizeof value;
    if (RegGetValueA(HKEY_CURRENT_USER, "Software\\Electronic Arts\\The Sims 25", name, RRF_RT_REG_SZ, nullptr,
                     value, &size) != ERROR_SUCCESS)
        return "";
    return value;
}

}  // namespace util
