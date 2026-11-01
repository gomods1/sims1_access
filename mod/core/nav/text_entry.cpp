#include "text_entry.h"

#include <string>

#include "../game/rtti.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/strings.h"
#include "../util/text.h"

namespace nav::text {
namespace {

// A typed character (game text, one or two bytes) as speech: a space would be silent.
std::string Spoken(const std::string& character) {
    if (character == " ") return S(Str::Space);
    return util::GameTextToUtf8(character);
}

// The double-byte code pages (Japanese, Chinese, Korean) send a character as two WM_CHARs to the
// game's ANSI window: the first byte waits here for the second.
unsigned char g_leadByte = 0;

// A caret key went to the field: its result is spoken on the next tick.
struct PendingCaret {
    game::Ptr field = 0;
    bool word = false;
    int tries = 0;
} g_caret;

// The character starting at byte `at` of game text (one or two bytes); empty at the end.
std::string CharacterAt(const std::string& text, int at) {
    if (at < 0 || at >= (int)text.size()) return "";
    int n = util::IsGameLeadByte((unsigned char)text[at]) && at + 1 < (int)text.size() ? 2 : 1;
    return text.substr(at, n);
}

// What the caret is on: the character, or the word from it to the next space.
std::string AtCaret(game::Ptr field, bool word) {
    std::string text = game::TextFieldRaw(field);
    int caret = game::TextFieldCaret(field);
    std::string c = CharacterAt(text, caret);
    if (c.empty() || c == "\r" || c == "\n") return S(Str::Blank);
    if (!word || c == " ") return Spoken(c);
    size_t end = text.find_first_of(" \r\n", caret);
    return util::GameTextToUtf8(text.substr(caret, end == std::string::npos ? std::string::npos : end - caret));
}

}  // namespace

game::Ptr FocusedField() {
    game::Ptr field = game::KeyboardFocus();
    return game::IsKindOf(field, "cTSWinTextEdit2") ? field : 0;
}

void OnChar(WPARAM ch) {
    if (!FocusedField() || ch < 0x20 || ch >= 0x100) {
        g_leadByte = 0;
        return;
    }
    unsigned char byte = (unsigned char)ch;
    if (g_leadByte) {
        std::string character = {(char)g_leadByte, (char)byte};
        g_leadByte = 0;
        speech::Say(Spoken(character));
    } else if (util::IsGameLeadByte(byte)) {
        g_leadByte = byte;
    } else {
        speech::Say(Spoken(std::string(1, (char)byte)));
    }
}

void OnBackspace() {
    game::Ptr field = FocusedField();
    if (!field) return;
    std::string text = game::TextFieldRaw(field);
    int caret = game::TextFieldCaret(field);
    if (caret > 0 && caret <= (int)text.size()) speech::Say(Spoken(util::LastGameCharacter(text.substr(0, caret))));
}

void TakeTyping(uintptr_t field) {
    if (field && game::IsKindOf(field, "cTSWinTextEdit2") && game::KeyboardFocus() != field) game::FocusTextField(field);
}

bool OnCaretKey(WPARAM vk, bool ctrl, uintptr_t focusedWidget) {
    game::Ptr field = FocusedField();
    if (!field || field != focusedWidget) return false;
    switch (vk) {
    case VK_LEFT: case VK_RIGHT: case VK_HOME: case VK_END:
        g_caret = PendingCaret{field, ctrl && (vk == VK_LEFT || vk == VK_RIGHT), 0};
        return true;
    case VK_DELETE: {
        if (ctrl) return false;
        std::string text = game::TextFieldRaw(field);
        std::string c = CharacterAt(text, game::TextFieldCaret(field));
        speech::Say(c.empty() ? std::string(S(Str::Blank)) : Spoken(c));
        return true;
    }
    default:
        return false;
    }
}

void Tick() {
    if (!g_caret.field) return;
    if (++g_caret.tries < 2) return;  // the game handles the key after the mod's hook
    game::Ptr field = g_caret.field;
    bool word = g_caret.word;
    g_caret = PendingCaret{};
    if (FocusedField() == field) speech::Say(AtCaret(field, word));
}

}  // namespace nav::text
