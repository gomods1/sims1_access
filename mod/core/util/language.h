// The language the game runs in: which translation of the mod's strings to speak (util/strings.h)
// and how the game encodes its own text.
//
// The game picks its language at startup (0x457600): the registry value SIMS_LANGUAGE (which Steam
// writes from the game's language in Steam), then Steam's language itself, then a -l<Language>
// command-line switch (-lGerman, -lJapanese...), each overriding the one before. It keeps the
// result as a language number (the STR# language byte): 1 US English, 2 UK English, 3 French,
// 4 German, 5 Italian, 6 Spanish, 7 Dutch, 8 Danish, 9 Swedish, 10 Norwegian, 11 Finnish,
// 12 Hebrew and 13 Russian (slots holding English text), 14 Portuguese (Brazil), 15 Japanese,
// 16 Polish, 17 Simplified Chinese, 18 Traditional Chinese, 19 Thai, 20 Korean.
#pragma once

#include <string>

namespace util {

enum class Language {
    English,
    French,
    German,
    Italian,
    Spanish,
    Dutch,
    Danish,
    Swedish,
    Norwegian,
    Finnish,
    Portuguese,
    Japanese,
    Polish,
    SimplifiedChinese,
    TraditionalChinese,
    Thai,
    Korean,
};

// Before game memory can be read: the language the registry names, else Windows' own.
void InitLanguage();

// The game's language number once the game can be asked (game::LanguageNumber()): it wins over
// the registry, which Steam's language or a switch may have overridden. 0 (unknown) is ignored.
void SetGameLanguage(int number);

Language CurrentLanguage();

// The Windows code page of the game's text in the current language (1252, 1250, 932...).
unsigned GameCodePage();

// What separates thousands in amounts of money the mod writes: "1,200", "1.200", or nothing
// where the language groups digits with spaces (a synthesizer can read "1 200" as two numbers).
const char* DigitGroupSeparator();

}  // namespace util
