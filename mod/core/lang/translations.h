// The mod's strings in the languages the game ships, one file per language in this folder.
// English is the source (util/strings_table.h); a string a language lacks is spoken in English.
// tools/check_strings.py checks every file against the English table (ids, placeholders).
#pragma once

#include <cstddef>

#include "../util/strings.h"

namespace lang {

struct Translation {
    Str id;
    const char* text;  // UTF-8, with the English string's printf placeholders
};

struct Translations {
    const Translation* entries;
    size_t count;
};

extern const Translations kFrench;
extern const Translations kGerman;
extern const Translations kItalian;
extern const Translations kSpanish;
extern const Translations kDutch;
extern const Translations kDanish;
extern const Translations kSwedish;
extern const Translations kNorwegian;
extern const Translations kFinnish;
extern const Translations kPortuguese;
extern const Translations kJapanese;
extern const Translations kPolish;
extern const Translations kSimplifiedChinese;
extern const Translations kTraditionalChinese;
extern const Translations kThai;
extern const Translations kKorean;

}  // namespace lang
