#include "settings.h"

#include <windows.h>

#include <string>

#include "log.h"

namespace util {
namespace {

std::wstring g_path;

std::wstring Wide(const char* s) {
    std::wstring w;
    for (; *s; s++) w += (wchar_t)(unsigned char)*s;
    return w;
}

}  // namespace

void InitSettings(const wchar_t* modDir) { g_path = std::wstring(modDir) + L"\\SimsAccess.ini"; }

int GetSetting(const char* key, int fallback) {
    if (g_path.empty()) return fallback;
    return (int)GetPrivateProfileIntW(L"mod", Wide(key).c_str(), fallback, g_path.c_str());
}

void SetSetting(const char* key, int value) {
    if (g_path.empty()) return;
    if (!WritePrivateProfileStringW(L"mod", Wide(key).c_str(), std::to_wstring(value).c_str(), g_path.c_str()))
        Log("settings: could not write %s (error %lu)", key, GetLastError());
}

}  // namespace util
