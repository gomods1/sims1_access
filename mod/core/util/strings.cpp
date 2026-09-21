#include "strings.h"

#include <windows.h>

#include "game_settings.h"
#include "log.h"

namespace util {
namespace {

struct Entry {
    const char* en;
    const char* es;
};

const Entry kStrings[] = {
#define SA_STRING_ENTRY(id, en, es) {en, es},
    SA_STRINGS(SA_STRING_ENTRY)
#undef SA_STRING_ENTRY
};
static_assert(sizeof(kStrings) / sizeof(kStrings[0]) == (size_t)Str::Count, "string table size");

bool g_spanish = true;

}  // namespace

void InitStrings() {
    std::string language = ReadGameSetting("SIMS_LANGUAGE");
    if (!language.empty()) {
        g_spanish = _stricmp(language.c_str(), "Spanish") == 0;
        Log("game language: %s", language.c_str());
    } else {
        g_spanish = PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_SPANISH;
        Log("SIMS_LANGUAGE not found; mod strings in %s", g_spanish ? "Spanish" : "English");
    }
}

bool LanguageIsSpanish() { return g_spanish; }

const char* Lookup(Str id) {
    const Entry& e = kStrings[(int)id];
    return g_spanish ? e.es : e.en;
}

}  // namespace util

const char* S(Str id) { return util::Lookup(id); }
