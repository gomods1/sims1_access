// Text helpers. The game's text is in its language's code page (util/language.h: 1252 for the
// Western languages, 1250 Polish, 932 Japanese...); everything the mod speaks is UTF-8.
#pragma once

#include <string>

namespace util {

// Game text to UTF-8, and back (the cheat console's input).
std::string GameTextToUtf8(const std::string& s);
std::string Utf8ToGameText(const std::string& utf8);

// The first byte of a two-byte character in the game's code page (Japanese, Chinese, Korean).
bool IsGameLeadByte(unsigned char c);

// The last character of game text, one or two bytes; empty for empty text.
std::string LastGameCharacter(const std::string& s);

// Line breaks become a spoken pause (". " unless the line already ends in punctuation), runs of
// whitespace collapse to one space, control characters are dropped. UTF-8 in and out.
std::string CleanForSpeech(const std::string& utf8);

// Drops a trailing hotkey hint from a game label: "Velocidad normal - 1" -> "Velocidad normal",
// "Pausa - P" -> "Pausa". Labels without one ("Ver de cerca +") are returned unchanged.
std::string WithoutHotkey(const std::string& label);

// Money: "$1,200" in English, "$1.200" in Spanish (util::DigitGroupSeparator).
std::string Money(int amount);

// printf into a std::string, of any length. Positional arguments (%2$s) work, so a translation
// can put a string's values in another order.
std::string Format(const char* fmt, ...);

}  // namespace util
