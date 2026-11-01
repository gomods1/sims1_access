#include "ui.h"

#include "../util/log.h"
#include "../util/text.h"
#include "addresses.h"
#include "rtti.h"

namespace game {
namespace {

Ptr WinMgr() { return Rd<Ptr>(Va(addr::kWinMgr)); }

typedef char(__thiscall* HandleMessageFn)(void* widget, uint32_t* message);

// Delivers {type, x, y, flags} to the widget's own message handler (vtable slot 0).
bool SendMessageTo(Ptr w, uint32_t type, int x, int y, uint32_t flags) {
    Ptr vtable = Rd<Ptr>(w);
    Ptr handler = Rd<Ptr>(vtable);
    if (!InImage(vtable) || !InImage(handler)) return false;
    uint32_t message[4] = {type, (uint32_t)x, (uint32_t)y, flags};
    if (!Guarded([&] { ((HandleMessageFn)handler)((void*)w, message); })) {
        Log("widget message %u crashed on %p", type, (void*)w);
        return false;
    }
    return true;
}

}  // namespace

// The window manager's input window can be a panel inside a screen (the family panel after a
// click); the root is its top-level ancestor below the main window. Modal dialogs are direct
// children of the main window; pie menus have no parent at all.
Ptr InputRoot() {
    Ptr main = MainWindow();
    Ptr w = Rd<Ptr>(WinMgr() + addr::winmgr::kInputRoot);
    if (!IsKindOf(w, "cTSWin")) return main;
    for (int depth = 0; depth < 32; depth++) {
        Ptr parent = Parent(w);
        if (!parent || parent == main || !IsKindOf(parent, "cTSWin")) break;
        w = parent;
    }
    return w;
}

Ptr KeyboardFocus() { return Rd<Ptr>(WinMgr() + addr::winmgr::kInputRoot); }

std::string TextFieldRaw(Ptr edit) {
    std::string text = ReadStdString(edit + addr::textedit::kText);
    while (!text.empty() && text.back() == '\0') text.pop_back();
    return text;
}

std::string TextFieldText(Ptr edit) { return util::GameTextToUtf8(TextFieldRaw(edit)); }

int TextFieldCaret(Ptr edit) { return Rd<int>(edit + addr::textedit::kCaret); }

bool FocusTextField(Ptr edit) {
    typedef Ptr(__thiscall * SetInputFn)(void*, void*);
    Ptr mgr = WinMgr();
    if (!mgr || !IsKindOf(edit, "cTSWinTextEdit2") || !Visible(edit)) return false;
    if (KeyboardFocus() == edit) return true;
    if (!FunctionOk(addr::winmgr::kSetInput)) return false;
    bool ok = Guarded([&] { ((SetInputFn)Va(addr::winmgr::kSetInput))((void*)mgr, (void*)edit); });
    Log("text field %p takes typing: %s", (void*)edit, ok && KeyboardFocus() == edit ? "yes" : "no");
    return ok && KeyboardFocus() == edit;
}

Ptr MainWindow() { return Rd<Ptr>(WinMgr() + addr::winmgr::kMainWindow); }

void ForceAppActive() { WriteU8(Va(addr::kAppActive), 1); }

std::vector<Ptr> Children(Ptr w) {
    std::vector<Ptr> out;
    Ptr head = Rd<Ptr>(w + addr::win::kChildren);
    uint32_t count = Rd<uint32_t>(w + addr::win::kChildren + 4);
    if (!head || count == 0 || count > 4000) return out;
    for (Ptr node = Rd<Ptr>(head); node && node != head && out.size() < count; node = Rd<Ptr>(node)) {
        Ptr child = Rd<Ptr>(node + 8);  // node: {next, prev, value}
        if (child) out.push_back(child);
    }
    return out;
}

Ptr Parent(Ptr w) { return Rd<Ptr>(w + addr::win::kParent); }

RECT Rect(Ptr w) {
    RECT r = {};
    Read(w + addr::win::kRect, &r, sizeof r);
    return r;
}

bool Visible(Ptr w) { return (Rd<uint32_t>(w + addr::win::kFlags) & addr::win::kFlagVisible) != 0; }

std::string Caption(Ptr w) { return ReadStdStringUtf8(w + addr::win::kCaption); }
std::string Tooltip(Ptr w) { return ReadStdStringUtf8(w + addr::win::kTooltip); }

bool ButtonEnabled(Ptr b) {
    return Rd<uint8_t>(b + addr::btn::kEnabled) != 0 && (Rd<uint32_t>(b + addr::win::kFlags) & addr::win::kFlagEnabled) != 0;
}
ButtonType GetButtonType(Ptr b) { return (ButtonType)Rd<int>(b + addr::btn::kType); }
bool ButtonChecked(Ptr b) { return Rd<int>(b + addr::btn::kChecked) != 0; }

Ptr CheckBoxButton(Ptr check) {
    for (Ptr c : Children(check))
        if (IsKindOf(c, "cTSWinBtn")) return c;
    return 0;
}

bool CheckBoxChecked(Ptr check) {
    Ptr b = CheckBoxButton(check);
    return b && ButtonChecked(b);
}

SliderState ReadSlider(Ptr slider) {
    SliderState s;
    s.value = Rd<int>(slider + addr::slider::kValue);
    s.min = Rd<int>(slider + addr::slider::kMin);
    s.max = Rd<int>(slider + addr::slider::kMax);
    return s;
}

bool SetSlider(Ptr slider, int value) {
    typedef void(__thiscall * SetFn)(void*, int);
    Ptr fn = Rd<Ptr>(Rd<Ptr>(slider) + addr::kSliderSetValueSlot * 4);
    if (!IsKindOf(slider, "cTSWinSlider") || fn != Va(addr::kSliderSetValue) || !FunctionOk(addr::kSliderSetValue))
        return false;
    return Guarded([&] { ((SetFn)fn)((void*)slider, value); });
}

BudgetRow ReadBudgetRow(Ptr row) {
    BudgetRow r;
    r.column1 = ReadStdStringUtf8(row + addr::budget::kColumn1);
    r.column2 = ReadStdStringUtf8(row + addr::budget::kColumn2);
    std::string format = ReadStdStringUtf8(row + addr::budget::kSummaryFormat);
    std::string value = ReadStdStringUtf8(row + addr::budget::kSummaryValue);
    size_t slot = format.find("%s");
    if (slot != std::string::npos) format.replace(slot, 2, value);
    r.summary = format;
    return r;
}

bool Press(Ptr w) {
    RECT r = Rect(w);
    int x = (r.right - r.left) / 2, y = (r.bottom - r.top) / 2;
    return SendMessageTo(w, addr::msg::kMouseEnter, x, y, 0) &&
           SendMessageTo(w, addr::msg::kLeftDown, x, y, addr::msg::kLeftButtonFlag) &&
           SendMessageTo(w, addr::msg::kLeftUp, x, y, 0);
}

bool PressAt(Ptr w, int x, int y) {
    return SendMessageTo(w, addr::msg::kMouseEnter, x, y, 0) &&
           SendMessageTo(w, addr::msg::kMouseMove, x, y, 0) &&
           SendMessageTo(w, addr::msg::kLeftDown, x, y, addr::msg::kLeftButtonFlag) &&
           SendMessageTo(w, addr::msg::kLeftUp, x, y, 0);
}

void Hover(Ptr w) {
    RECT r = Rect(w);
    int x = (r.right - r.left) / 2, y = (r.bottom - r.top) / 2;
    SendMessageTo(w, addr::msg::kMouseEnter, x, y, 0) && SendMessageTo(w, addr::msg::kMouseMove, x, y, 0);
}

void Unhover(Ptr w) {
    RECT r = Rect(w);
    SendMessageTo(w, addr::msg::kMouseLeave, (r.right - r.left) / 2, (r.bottom - r.top) / 2, 0);
}

}  // namespace game
