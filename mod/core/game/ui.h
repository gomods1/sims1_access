// The game's UI toolkit (cTSWin widgets and the cTSWinMgr window manager): the widget tree, what
// each widget shows, and pressing widgets without a mouse.
#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "memory.h"

namespace game {

// Window manager
Ptr InputRoot();    // the screen or modal dialog that owns input now
Ptr MainWindow();

// Development: the game drops input while its "app active" byte is 0 (window in the
// background). Setting it lets tests drive the game behind other windows. Never for players.
void ForceAppActive();

// Widgets (cTSWin)
std::vector<Ptr> Children(Ptr w);
Ptr Parent(Ptr w);
RECT Rect(Ptr w);             // absolute, 1024x768 game space
bool Visible(Ptr w);          // the widget's own visibility flag
std::string Caption(Ptr w);   // UTF-8
std::string Tooltip(Ptr w);   // UTF-8

// Buttons (cTSWinBtn)
enum class ButtonType { Push = 0, Toggle = 1, Check = 2 };
bool ButtonEnabled(Ptr b);
ButtonType GetButtonType(Ptr b);
bool ButtonChecked(Ptr b);

// Text fields (cTSWinTextEdit2)
Ptr KeyboardFocus();                  // the widget typing goes to (a text field while one is active)
std::string TextFieldText(Ptr edit);  // UTF-8
std::string TextFieldRaw(Ptr edit);   // cp1252, one byte per character
int TextFieldCaret(Ptr edit);

// Check boxes (cTSWinCheck): a container holding the actual tick button; the box's name is a
// separate label button next to it.
Ptr CheckBoxButton(Ptr check);  // the tick button, 0 if none
bool CheckBoxChecked(Ptr check);

// Sliders (cTSWinSlider): volume and similar settings.
struct SliderState {
    int value = 0, min = 0, max = 0;
};
SliderState ReadSlider(Ptr slider);
bool SetSlider(Ptr slider, int value);  // as if dragged: the owner applies it

// Budget dialog rows (cWinBudgetRow): the caption is the row's label or a section title.
struct BudgetRow {
    std::string column1, column2;  // per-period values, or the column names on the header row
    std::string summary;           // a summary line with its value filled in
};
BudgetRow ReadBudgetRow(Ptr row);

// Presses a widget the way a mouse click does (enter, left down, left up, through its own
// message handler), so sounds, state changes and notifications all run. The press may close or
// destroy the widget; call Unhover(w) afterwards only once w is known to still exist.
bool Press(Ptr w);
// The same at a point inside the widget (relative), with the pointer moved there first: for
// widgets whose click depends on where it lands (a personality bar).
bool PressAt(Ptr w, int x, int y);
void Unhover(Ptr w);

}  // namespace game
