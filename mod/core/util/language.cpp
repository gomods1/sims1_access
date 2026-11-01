#include "language.h"

#include <windows.h>

#include "../lang/translations.h"
#include "game_settings.h"
#include "log.h"
#include "strings.h"

namespace util {
namespace {

struct LanguageInfo {
    Language language;
    const char* name;                    // for the log
    unsigned codePage;                   // the game's text in this language
    const char* digitGroupSeparator;
    const lang::Translations* strings;   // nullptr: English, the source
    const char* docTag;                  // the manual's file: SimsAccess\docs\<tag>.html
};

// Code pages verified on the game's UIText.iff (tools/simsfmt.py). Thai text joins its words
// with '_', which the game does not draw (see GameTextToUtf8).
const LanguageInfo kLanguages[] = {
    {Language::English, "English", 1252, ",", nullptr, "en"},
    {Language::French, "French", 1252, "", &lang::kFrench, "fr"},
    {Language::German, "German", 1252, ".", &lang::kGerman, "de"},
    {Language::Italian, "Italian", 1252, ".", &lang::kItalian, "it"},
    {Language::Spanish, "Spanish", 1252, ".", &lang::kSpanish, "es"},
    {Language::Dutch, "Dutch", 1252, ".", &lang::kDutch, "nl"},
    {Language::Danish, "Danish", 1252, ".", &lang::kDanish, "da"},
    {Language::Swedish, "Swedish", 1252, "", &lang::kSwedish, "sv"},
    {Language::Norwegian, "Norwegian", 1252, "", &lang::kNorwegian, "no"},
    {Language::Finnish, "Finnish", 1252, "", &lang::kFinnish, "fi"},
    {Language::Portuguese, "Portuguese (Brazil)", 1252, ".", &lang::kPortuguese, "pt"},
    {Language::Japanese, "Japanese", 932, ",", &lang::kJapanese, "ja"},
    {Language::Polish, "Polish", 1250, "", &lang::kPolish, "pl"},
    {Language::SimplifiedChinese, "Simplified Chinese", 936, ",", &lang::kSimplifiedChinese, "zh-hans"},
    {Language::TraditionalChinese, "Traditional Chinese", 950, ",", &lang::kTraditionalChinese, "zh-hant"},
    {Language::Thai, "Thai", 874, ",", &lang::kThai, "th"},
    {Language::Korean, "Korean", 949, ",", &lang::kKorean, "ko"},
};

// The game's language numbers, with the SIMS_LANGUAGE value that selects each (0x457600).
// Hebrew and Russian have no registry name the game accepts and hold English text.
struct GameLanguage {
    int number;
    const char* setting;
    Language language;
};

const GameLanguage kGameLanguages[] = {
    {1, "USEnglish", Language::English},
    {2, "UKEnglish", Language::English},
    {3, "French", Language::French},
    {4, "German", Language::German},
    {5, "Italian", Language::Italian},
    {6, "Spanish", Language::Spanish},
    {7, "Dutch", Language::Dutch},
    {8, "Danish", Language::Danish},
    {9, "Swedish", Language::Swedish},
    {10, "Norwegian", Language::Norwegian},
    {11, "Finnish", Language::Finnish},
    {12, nullptr, Language::English},
    {13, nullptr, Language::English},
    {14, "Portuguese", Language::Portuguese},
    {15, "Japanese", Language::Japanese},
    {16, "Polish", Language::Polish},
    {17, "SimplifiedChinese", Language::SimplifiedChinese},
    {18, "TraditionalChinese", Language::TraditionalChinese},
    {19, "Thai", Language::Thai},
    {20, "Korean", Language::Korean},
};

// Windows' UI language, for a game without its registry settings.
Language WindowsLanguage() {
    switch (PRIMARYLANGID(GetUserDefaultUILanguage())) {
    case LANG_FRENCH: return Language::French;
    case LANG_GERMAN: return Language::German;
    case LANG_ITALIAN: return Language::Italian;
    case LANG_SPANISH: return Language::Spanish;
    case LANG_DUTCH: return Language::Dutch;
    case LANG_DANISH: return Language::Danish;
    case LANG_SWEDISH: return Language::Swedish;
    case LANG_NORWEGIAN: return Language::Norwegian;
    case LANG_FINNISH: return Language::Finnish;
    case LANG_PORTUGUESE: return Language::Portuguese;
    case LANG_JAPANESE: return Language::Japanese;
    case LANG_POLISH: return Language::Polish;
    case LANG_CHINESE:
        return SUBLANGID(GetUserDefaultUILanguage()) == SUBLANG_CHINESE_SIMPLIFIED ? Language::SimplifiedChinese
                                                                                   : Language::TraditionalChinese;
    case LANG_THAI: return Language::Thai;
    case LANG_KOREAN: return Language::Korean;
    default: return Language::English;
    }
}

const LanguageInfo* g_current = &kLanguages[0];

const LanguageInfo& InfoOf(Language language) {
    for (const LanguageInfo& info : kLanguages)
        if (info.language == language) return info;
    return kLanguages[0];
}

void Use(Language language, const char* source) {
    g_current = &InfoOf(language);
    LoadStrings(g_current->strings);
    Log("language: %s (%s), code page %u", g_current->name, source, g_current->codePage);
}

}  // namespace

void InitLanguage() {
    std::string setting = ReadGameSetting("SIMS_LANGUAGE");
    for (const GameLanguage& g : kGameLanguages) {
        if (g.setting && _stricmp(setting.c_str(), g.setting) == 0) {
            Use(g.language, "registry");
            return;
        }
    }
    Use(WindowsLanguage(), setting.empty() ? "no SIMS_LANGUAGE, Windows' language" : "unknown SIMS_LANGUAGE");
}

void SetGameLanguage(int number) {
    if (number == 0) return;
    for (const GameLanguage& g : kGameLanguages) {
        if (g.number != number) continue;
        if (g.language != g_current->language) Use(g.language, "the game's choice");
        return;
    }
    Log("unknown game language number %d; keeping %s", number, g_current->name);
}

Language CurrentLanguage() { return g_current->language; }

const char* DocTag() { return g_current->docTag; }

unsigned GameCodePage() { return g_current->codePage; }

const char* DigitGroupSeparator() { return g_current->digitGroupSeparator; }

}  // namespace util
