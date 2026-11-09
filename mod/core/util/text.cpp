#include "text.h"

#include "strings.h"

#include <windows.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <vector>

#include "language.h"

namespace util {

std::string FoldForSearch(const std::string& utf8) {
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (n <= 0) return "";
    std::wstring wide(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], n);
    // Accents apart from their letters, then without them.
    int m = FoldStringW(MAP_COMPOSITE, wide.c_str(), n, nullptr, 0);
    std::wstring parts(m > 0 ? m : 0, L'\0');
    if (m > 0) FoldStringW(MAP_COMPOSITE, wide.c_str(), n, &parts[0], m);
    std::vector<WORD> types(parts.size());
    std::wstring bare;
    if (!parts.empty() && GetStringTypeW(CT_CTYPE3, parts.c_str(), (int)parts.size(), types.data()))
        for (size_t i = 0; i < parts.size(); i++)
            if (!(types[i] & (C3_NONSPACING | C3_DIACRITIC))) bare += parts[i];
    if (bare.empty()) return "";
    CharLowerBuffW(&bare[0], (DWORD)bare.size());
    int out = WideCharToMultiByte(CP_UTF8, 0, bare.c_str(), (int)bare.size(), nullptr, 0, nullptr, nullptr);
    std::string folded(out, '\0');
    WideCharToMultiByte(CP_UTF8, 0, bare.c_str(), (int)bare.size(), &folded[0], out, nullptr, nullptr);
    return folded;
}
namespace {

std::string Convert(const std::string& s, unsigned fromCodePage, unsigned toCodePage) {
    if (s.empty()) return s;
    int wide = MultiByteToWideChar(fromCodePage, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(wide, L'\0');
    MultiByteToWideChar(fromCodePage, 0, s.data(), (int)s.size(), w.data(), wide);
    int n = WideCharToMultiByte(toCodePage, 0, w.data(), wide, nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(toCodePage, 0, w.data(), wide, out.data(), n, nullptr, nullptr);
    return out;
}

// Where the character after the one at `i` starts.
size_t NextCharacter(const std::string& s, size_t i) {
    return i + (IsGameLeadByte((unsigned char)s[i]) && i + 1 < s.size() ? 2 : 1);
}

}  // namespace

std::string GameTextToUtf8(const std::string& s) {
    std::string text = s;
    // Thai text joins its words with '_': where a line may break, not drawn by the game's font.
    if (CurrentLanguage() == Language::Thai) text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
    return Convert(text, GameCodePage(), CP_UTF8);
}

std::string Utf8ToGameText(const std::string& utf8) { return Convert(utf8, CP_UTF8, GameCodePage()); }

bool IsGameLeadByte(unsigned char c) { return IsDBCSLeadByteEx(GameCodePage(), c) != 0; }

std::string LastGameCharacter(const std::string& s) {
    // Only a walk from the start tells characters apart: a second byte can look like a first one.
    size_t last = 0;
    for (size_t i = 0; i < s.size(); i = NextCharacter(s, i)) last = i;
    return s.substr(last);
}

namespace {

bool EndsWithPunctuation(const std::string& s) {
    if (s.empty()) return false;
    switch (s.back()) {
    case '.': case ',': case ':': case ';': case '!': case '?': return true;
    default: return false;
    }
}

std::string CollapseSpaces(const std::string& line) {
    std::string out;
    bool pendingSpace = false;
    for (char c : line) {
        if (c == ' ' || c == '\t') {
            pendingSpace = !out.empty();
            continue;
        }
        if (pendingSpace) out += ' ';
        pendingSpace = false;
        out += c;
    }
    return out;
}

}  // namespace

std::string CleanForSpeech(const std::string& in) {
    std::vector<std::string> lines(1);
    for (char c : in) {
        if (c == '\r' || c == '\n') lines.emplace_back();
        else lines.back() += (unsigned char)c < 0x20 ? ' ' : c;
    }
    std::string out;
    for (const std::string& raw : lines) {
        std::string line = CollapseSpaces(raw);
        if (line.empty()) continue;
        if (!out.empty()) out += EndsWithPunctuation(out) ? " " : ". ";
        out += line;
    }
    return out;
}

std::string WithoutHotkey(const std::string& label) {
    size_t dash = label.rfind(" - ");
    if (dash == std::string::npos || dash == 0) return label;
    size_t hotkeyLength = label.size() - (dash + 3);
    return hotkeyLength >= 1 && hotkeyLength <= 3 ? label.substr(0, dash) : label;
}

std::string Money(int amount) {
    std::string digits = std::to_string(amount < 0 ? -amount : amount);
    const char* separator = DigitGroupSeparator();
    for (int i = (int)digits.size() - 3; i > 0; i -= 3) digits.insert(i, separator);
    return (amount < 0 ? "-$" : "$") + digits;
}

std::string SpokenMoney(const std::string& s) {
    if (s.find('$') == std::string::npos) return s;
    auto digit = [&](size_t i) { return i < s.size() && s[i] >= '0' && s[i] <= '9'; };
    std::string out;
    for (size_t i = 0; i < s.size();) {
        if (s[i] != '$' || !digit(i + 1)) {
            out += s[i++];
            continue;
        }
        size_t j = i + 1;
        while (j < s.size()) {
            if (digit(j)) {
                j++;
                continue;
            }
            // a group separator: . , ' space or a no-break space, then exactly three digits
            size_t sep = (s[j] == '.' || s[j] == ',' || s[j] == ' ' || s[j] == '\'') ? 1
                         : s.compare(j, 2, "\xc2\xa0") == 0 ? 2
                                                             : 0;
            if (sep && digit(j + sep) && digit(j + sep + 1) && digit(j + sep + 2) && !digit(j + sep + 3)) {
                j += sep + 3;
                continue;
            }
            break;
        }
        out += Format(S(Str::MoneySpoken), s.substr(i + 1, j - i - 1).c_str());
        i = j;
    }
    return out;
}

std::string Format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int length = _vscprintf_p(fmt, ap);
    va_end(ap);
    if (length <= 0) return "";
    std::string out(length + 1, '\0');
    va_start(ap, fmt);
    _vsprintf_p(out.data(), out.size(), fmt, ap);
    va_end(ap);
    out.resize(length);
    return out;
}

}  // namespace util
