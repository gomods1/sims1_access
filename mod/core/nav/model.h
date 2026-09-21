// What the navigator moves over: items (navigable widgets) in groups (their container).
#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "../game/memory.h"

namespace nav {

using game::Ptr;

// How an item reads and behaves. Decided once when the widget tree is collected.
enum class Role {
    Button,        // any pressable widget with a label (tooltip or caption)
    Text,          // read-only text: dialog messages, titles
    Lot,           // a house on the neighborhood map
    FamilyRow,     // a family in the family picker
    Motive,        // a need meter
    QueuedAction,  // an icon in the selected Sim's action queue (pressing cancels it)
    TutorialIcon,  // the help system's "new tip" icon
    PieItem,       // an entry of the interaction (pie) menu; number = index in the menu
    BudgetRow,     // a row of the family budget: label, per-period values or a summary
    Status,        // lot view status line: clock, funds, speed, friends (number = StatusField)
    FamilyMember,  // a Sim's portrait on the live mode family panel (pressing selects the Sim)
    Mood,          // the selected Sim's mood meter
    Relationship,  // a card on the relationships subpanel: daily and lifetime scores
    Skill,         // a skill on the job subpanel (number = skill slot)
    JobInfo,       // a line of the job subpanel (number = JobField)
    Trait,         // a personality trait: name and points out of 10
    Interest,      // an interest: name and points out of 10
    HouseRating,   // a rating on the house subpanel (number = rating index)
    Info,          // read-only text shown on a button-like widget ("Dormitorios: 1")
    TextField,     // a text field; label = the prompt next to it ("Introduce apellido:")
    TraitBar,      // create a Sim: a personality bar, adjusted with PageUp/PageDown
    SimPreview,    // create a Sim: the preview of the Sim (reads the head and body shown)
    PickerMember,  // family picker: a member of the selected family (number = neighbor ID)
    CheckBox,      // a check box (cTSWinCheck); label = the label button on its row
    Slider,        // a slider (cTSWinSlider); label = the label left of it; PageUp/PageDown move it
    Spinner,       // a value between two arrow buttons ("< Alta >"); widget = the value text,
                   // label = the name above it; PageUp/PageDown press the arrows
    Product,       // a buy mode catalog product (ProductButton): name, price, ratings
};

enum class JobField { Job, Pay, Performance, FriendsNeeded };

// What a Role::Status item reads. The clock item's widget is the control panel itself; the
// others are the panel's buttons for that value.
enum class StatusField { Clock, Funds, Speed, Friends };

struct Item {
    Ptr widget = 0;
    Role role = Role::Button;
    std::string label;  // UTF-8 as collected (tooltip, caption); state is added when described
    RECT rect = {};
    Ptr group = 0;      // group key when it is not the widget's container
    Ptr previous = 0;   // Role::Spinner: its arrow buttons
    Ptr next = 0;
    int number = 0;     // Role::Lot: house number (sort key); Role::PieItem: index;
                        // Role::Status: StatusField
};

// Identifies an item across refreshes. Items can share a widget (pie menu entries; the pause
// button is both a control and the status line's speed), so role and number count too.
struct ItemId {
    Ptr widget = 0;
    Role role = Role::Button;
    int number = 0;
    bool operator==(const ItemId& o) const { return widget == o.widget && role == o.role && number == o.number; }
    bool operator!=(const ItemId& o) const { return !(*this == o); }
    explicit operator bool() const { return widget != 0; }
};

inline ItemId IdOf(const Item& it) { return ItemId{it.widget, it.role, it.number}; }

// Read-only items: Enter repeats them instead of pressing anything.
inline bool IsReadOnly(const Item& it) {
    return it.role == Role::Text || it.role == Role::Info ||
           (it.role == Role::Status && it.number == (int)StatusField::Clock);
}

struct Group {
    Ptr key = 0;        // the container widget, or kHousesGroup
    std::vector<Item> items;
    RECT bounds = {};
};

// Groups gathered by meaning rather than by container. All houses of the neighborhood map form
// one group; the lot view's status line, family portraits and action queue get groups of their
// own (the queue's icons share their container with unrelated texts such as the FPS counter).
constexpr Ptr kHousesGroup = 1;
constexpr Ptr kStatusGroup = 2;
constexpr Ptr kFamilyGroup = 3;
constexpr Ptr kActionQueueGroup = 4;
constexpr Ptr kSimPanelsGroup = 5;     // the family panel's subpanel toggles, in the game's order
constexpr Ptr kJobGroup = 6;           // the job subpanel: job info, then skills
constexpr Ptr kHouseGroup = 7;         // the house subpanel: ratings, then area, rooms, lot
constexpr Ptr kPersonalityGroup = 8;   // create a Sim: the personality bars and the zodiac sign
constexpr Ptr kLooksGroup = 9;         // create a Sim: age, sex, skin, head and body
constexpr Ptr kMembersGroup = 10;      // family picker: the selected family's members

inline bool HasRole(const Group& g, Role role) {
    for (const Item& it : g.items)
        if (it.role == role) return true;
    return false;
}

inline bool IsTextOnly(const Group& g) {
    for (const Item& it : g.items)
        if (it.role != Role::Text) return false;
    return true;
}

// Index of the first item that is not text (what focus lands on when entering the group).
inline int FirstControl(const Group& g) {
    for (int i = 0; i < (int)g.items.size(); i++)
        if (g.items[i].role != Role::Text) return i;
    return 0;
}

}  // namespace nav
