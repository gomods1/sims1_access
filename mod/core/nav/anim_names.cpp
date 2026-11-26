#include "anim_names.h"

#include <windows.h>

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "../util/language.h"
#include "../util/log.h"

namespace nav::anims {
namespace {

std::wstring g_dir;  // the mod's folder + "anims"
std::unordered_map<std::string, std::string> g_texts;
std::string g_loadedTag;

bool LoadFile(const char* tag) {
    std::wstring path = g_dir;
    for (const char* c = tag; *c; c++) path += (wchar_t)*c;
    path += L".tsv";
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size = {};
    GetFileSizeEx(f, &size);
    std::string data((size_t)size.QuadPart, '\0');
    DWORD got = 0;
    ReadFile(f, data.data(), (DWORD)data.size(), &got, nullptr);
    CloseHandle(f);
    data.resize(got);
    size_t at = 0;
    while (at < data.size()) {
        size_t end = data.find('\n', at);
        if (end == std::string::npos) end = data.size();
        std::string line = data.substr(at, end - at);
        at = end + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;
        g_texts.emplace(line.substr(0, tab), line.substr(tab + 1));
    }
    Log("animation descriptions: %u from %s.tsv", (unsigned)g_texts.size(), tag);
    return true;
}

// The descriptions of the player's language, English for what it lacks.
void Load() {
    const char* tag = util::DocTag();
    if (g_loadedTag == tag) return;
    g_loadedTag = tag;
    g_texts.clear();
    LoadFile(tag);
    if (strcmp(tag, "en") != 0) LoadFile("en");  // emplace keeps the translation where there is one
}

char Kind(char c) {
    if (c >= '0' && c <= '9') return 'd';
    if (c >= 'A' && c <= 'Z') return 'u';
    if (c >= 'a' && c <= 'z') return 'l';
    return 0;
}

// The first part says who does it: \d*[a-z]{1,2}(\d*2[a-z]{1,2})? ("a", "a2o", "1a2a", "c2k").
bool IsWho(const std::string& part) {
    size_t i = 0, n = part.size();
    while (i < n && Kind(part[i]) == 'd') i++;
    size_t letters = 0;
    while (i < n && Kind(part[i]) == 'l') i++, letters++;
    if (letters < 1 || letters > 2) return false;
    if (i == n) return true;
    size_t digits = 0;
    while (i < n && Kind(part[i]) == 'd') i++, digits++;
    if (digits == 0 || part[i - 1] != '2') return false;
    letters = 0;
    while (i < n && Kind(part[i]) == 'l') i++, letters++;
    return letters >= 1 && letters <= 2 && i == n;
}

std::string Lower(std::string s) {
    for (char& c : s)
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
    return s;
}

}  // namespace

void Init(const wchar_t* modDir) {
    g_dir = modDir ? modDir : L"";
    if (!g_dir.empty() && g_dir.back() != L'\\') g_dir += L'\\';
    g_dir += L"anims\\";
    g_loadedTag.clear();
}

std::string Key(const std::string& name) {
    std::string raw = name;
    if (raw.size() >= 5 && Lower(raw.substr(raw.size() - 5)) == "_anim") raw.resize(raw.size() - 5);
    std::vector<std::string> parts;
    size_t at = 0;
    while (true) {
        size_t dash = raw.find('-', at);
        parts.push_back(raw.substr(at, dash == std::string::npos ? std::string::npos : dash - at));
        if (dash == std::string::npos) break;
        at = dash + 1;
    }
    size_t first = parts.size() > 1 && IsWho(Lower(parts[0])) ? 1 : 0;
    std::vector<std::string> out;
    auto add = [&](const std::string& w) {
        static const char* const kDrop[] = {"loop", "alt", "var", "fk", "ik", "blend"};
        if (w.empty() || Kind(w[0]) == 'd') return;
        for (const char* d : kDrop)
            if (w == d) return;
        if (w.size() > 1 && w[0] == 'v' && Kind(w[1]) == 'd') return;
        if (!out.empty() && out.back() == w) return;
        out.push_back(w);
    };
    for (size_t p = first; p < parts.size(); p++) {
        const std::string& s = parts[p];
        std::string cur;
        for (size_t i = 0; i < s.size(); i++) {
            char k = Kind(s[i]);
            if (!k) {
                add(Lower(cur));
                cur.clear();
                continue;
            }
            if (!cur.empty()) {
                char prev = Kind(s[i - 1]);
                char next = i + 1 < s.size() ? Kind(s[i + 1]) : 0;
                if ((k == 'd') != (prev == 'd') || (k == 'u' && prev == 'l') || (k == 'u' && prev == 'u' && next == 'l')) {
                    add(Lower(cur));
                    cur.clear();
                }
            }
            cur += s[i];
        }
        add(Lower(cur));
    }
    std::string key;
    for (const std::string& w : out) key += (key.empty() ? "" : " ") + w;
    return key;
}

std::string Spoken(const std::string& raw) {
    std::string key = Key(raw);
    if (key.empty()) return "";
    Load();
    auto it = g_texts.find(key);
    if (it != g_texts.end()) return it->second == "-" ? "" : it->second;
    // Not described: its words, but never the face's constant blinking or neutral look.
    if (key.compare(0, 5, "blink") == 0 || key.compare(0, 13, "no expression") == 0) return "";
    return key;
}

std::string Join(const std::vector<std::string>& descriptions) {
    // Clauses end at ", " (most languages), "、" (Japanese) or "，" (Chinese), kept with their
    // separator so a description keeps its own punctuation.
    static const char* const kSeps[] = {", ", "\xE3\x80\x81", "\xEF\xBC\x8C"};
    std::vector<std::string> said;
    std::string line;
    for (const std::string& d : descriptions) {
        std::string kept;
        size_t pos = 0;
        while (pos < d.size()) {
            size_t end = std::string::npos, sepLen = 0;
            for (const char* s : kSeps) {
                size_t f = d.find(s, pos);
                if (f < end) {
                    end = f;
                    sepLen = strlen(s);
                }
            }
            std::string clause = d.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
            std::string sep = end == std::string::npos ? "" : d.substr(end, sepLen);
            pos = end == std::string::npos ? d.size() : end + sepLen;
            if (clause.empty() || std::find(said.begin(), said.end(), clause) != said.end()) continue;
            said.push_back(clause);
            kept += clause + sep;
        }
        // A separator left at the end by a dropped last clause goes.
        for (const char* s : kSeps) {
            size_t n = strlen(s);
            if (kept.size() >= n && kept.compare(kept.size() - n, n, s) == 0) kept.resize(kept.size() - n);
        }
        if (!kept.empty()) line += (line.empty() ? "" : ", ") + kept;
    }
    return line;
}

}  // namespace nav::anims
