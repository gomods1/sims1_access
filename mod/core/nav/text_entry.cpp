#include "text_entry.h"

#include <string>

#include "../game/rtti.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/strings.h"
#include "../util/text.h"

namespace nav::text {
namespace {

// One cp1252 character as speech: a space would be silent.
std::string Spoken(unsigned char c) {
    if (c == ' ') return S(Str::Space);
    return util::Cp1252ToUtf8(std::string(1, (char)c));
}

}  // namespace

game::Ptr FocusedField() {
    game::Ptr field = game::KeyboardFocus();
    return game::IsKindOf(field, "cTSWinTextEdit2") ? field : 0;
}

void OnChar(WPARAM ch) {
    if (FocusedField() && ch >= 0x20 && ch < 0x100) speech::Say(Spoken((unsigned char)ch));
}

void OnBackspace() {
    game::Ptr field = FocusedField();
    if (!field) return;
    std::string text = game::TextFieldRaw(field);
    int caret = game::TextFieldCaret(field);
    if (caret > 0 && caret <= (int)text.size()) speech::Say(Spoken((unsigned char)text[caret - 1]));
}

}  // namespace nav::text
