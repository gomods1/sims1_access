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

}  // namespace nav::text
