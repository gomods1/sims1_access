#include "manual.h"

#include <windows.h>
#include <shellapi.h>

#include "../speech/speech.h"
#include "../util/language.h"
#include "../util/log.h"
#include "../util/strings.h"

namespace nav::manual {
namespace {

std::wstring g_docs;  // <mod dir>\docs\

std::wstring PageFor(const char* tag) {
    std::wstring path = g_docs;
    for (const char* c = tag; *c; c++) path += (wchar_t)*c;
    return path + L".html";
}

}  // namespace

void Init(const wchar_t* modDir) {
    g_docs = modDir ? modDir : L"";
    if (!g_docs.empty() && g_docs.back() != L'\\') g_docs += L'\\';
    g_docs += L"docs\\";
}

void Open() {
    const char* tag = util::DocTag();
    std::wstring page = PageFor(tag);
    if (GetFileAttributesW(page.c_str()) == INVALID_FILE_ATTRIBUTES) page = PageFor(tag = "en");
    if (GetFileAttributesW(page.c_str()) == INVALID_FILE_ATTRIBUTES) {
        Log("manual: no page in the docs folder");
        speech::Say(S(Str::ManualMissing));
        return;
    }
    speech::Say(S(Str::ManualOpening));
    // The browser takes the foreground; the game goes on behind it until the player comes back.
    HINSTANCE r = ShellExecuteW(nullptr, L"open", page.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    Log("manual: opened docs/%s.html (%d)", tag, (int)(INT_PTR)r);  // the path may not be ASCII
    if ((INT_PTR)r <= 32) speech::Say(S(Str::ManualMissing));
}

}  // namespace nav::manual
