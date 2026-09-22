#include "tutorial.h"

#include <windows.h>

#include <string>
#include <vector>

#include "../game/hooks.h"
#include "../game/lot.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "navigator.h"
#include "world.h"

namespace nav::tutorial {
namespace {

// The tutorial's step texts are the tutorial object's STR# 301 (game::TutorialTexts), the same
// entries in every language; a step is known by its index there. A step with a keyboard version
// is spoken as that instead of the mouse instructions; any other step as the game wrote it,
// followed by its keyboard hint if it has one. Indices as tools/simsfmt.py lists the table
// (0-based); some texts are repeated at a second index.
struct KeyboardStep {
    int index;
    Str text;
};

const KeyboardStep kSteps[] = {
    {38, Str::StepZoom},          // "Here is an overview of our Tutorial house. Press the zoom-in button..."
    {3, Str::StepZoomAgain},      // "This is the medium zoom level..."
    {5, Str::StepScroll},         // "...moving the cursor to different edges of the screen."
    {6, Str::StepScrollAgain},    // "...scroll is with the RIGHT mouse button..."
    {7, Str::StepRotate},         // "Now let's rotate the view..."
    {8, Str::StepLive},           // "...time to go into LIVE mode. Press the LIVE icon below."
    {10, Str::StepLive},
    {12, Str::StepPortrait},      // "...Click on Bob's head in the Control Panel..."
    {13, Str::StepGoHere},        // "...an empty spot on the floor... "Go Here"..."
    {14, Str::StepGoHereAgain},   // "Make sure you have the "foot" cursor..."
    {16, Str::StepInteract},      // "The Sims can be commanded to interact with most objects..."
    {18, Str::StepMood},          // "The Mood icon below is both an indicator and a button..."
    {20, Str::StepShower},        // "...command Bob to "Take a Shower.""
    {23, Str::StepNeeds},         // "Click on the different Need indicator bars..."
    {24, Str::StepBuy},           // "...Now let's explore BUY mode. Click the Buy button below."
    {26, Str::StepChooseChair},   // "Look through the Buy Catalog and choose a chair..."
    {27, Str::StepPlace},         // "To buy this object first move it to a clear spot..."
    {28, Str::StepTurn},          // "Good, now let's rotate it..."
    {30, Str::StepBackToLive},    // "Remember, you can always come back to BUY mode..."
    {31, Str::StepPersonality},   // "...Click on the Personality button below."
    {32, Str::StepPersonality},
    {56, Str::StepPersonalityTab},  // "Click on the personality traits..."
    {35, Str::StepSecondSim},     // "...her name is Betty. Click on her picture..."
    {37, Str::StepSecondObject},  // "Click on an object and make Betty do something..."
    {39, Str::StepSelectFirst},   // "...Move the cursor over Bob and RIGHT-click to select him."
    {40, Str::StepSpace},         // "...pressing the Space bar..."
    {41, Str::StepSocial},        // "...LEFT-click on Bob to make her interact with him..."
    {42, Str::StepSocialAgain},   // "...watching the relationship icon above their heads..."
    {43, Str::StepRelationships}, // "...by clicking on the Relationship button below."
    {47, Str::StepNewspaper},     // "Sims earn money by working... newspaper..."
    {49, Str::StepJobTab},        // "Congratulations! Now click on the Job button below..."
    {50, Str::StepJobName},       // "...Click on the job name to find out when the carpool will arrive."
    {51, Str::StepSkills},        // "Skills on the right (Cooking, Mechanical,...)..."
    {52, Str::StepOptions},       // "The Options mode is where you save, quit or return..."
};

struct Hint {
    int index;
    Str hint;
};

const Hint kHints[] = {
    {48, Str::HintNewspaper},  // "Look outside (near the mailbox) for the paper..."
};

struct State {
    std::string lastText;   // the step text last spoken
    RECT lastArrow = {};    // where the arrow was last announced
    bool arrowShown = false;
    bool tipShown = false;  // the help system's tip icon
    ULONGLONG tipHiddenAt = 0;  // when it last went away (it blinks)
    bool running = false;   // the tutorial object was on the lot at the last tick
    ULONGLONG escapeAt = 0; // the first Escape of a pair
};

State g;

// The index of the shown step text in the tutorial's texts, -1 when it is none of them. The panel
// shows the text as StepText cleans it, so the entries are compared the same way.
int StepIndex(const std::string& shown) {
    std::vector<std::string> texts = game::TutorialTexts();
    for (size_t i = 0; i < texts.size(); i++)
        if (util::CleanForSpeech(texts[i]) == shown) return (int)i;
    return -1;
}

const KeyboardStep* KeyboardStepAt(int index) {
    for (const KeyboardStep& s : kSteps)
        if (s.index == index) return &s;
    return nullptr;
}

const Hint* HintAt(int index) {
    for (const Hint& h : kHints)
        if (h.index == index) return &h;
    return nullptr;
}

// The step panel under the lot view, 0 when none is shown.
Ptr StepPanel(Ptr lotView) {
    for (Ptr c : game::Children(lotView))
        if (game::IsKindOf(c, "cTSObjectDialog") && game::Visible(c)) return c;
    return 0;
}

std::string StepText(Ptr panel) {
    std::string text;
    for (Ptr c : game::Children(panel))
        if (game::IsKindOf(c, "cWinTextWrap") || game::IsKindOf(c, "cTSWinText")) {
            std::string part = util::CleanForSpeech(game::Caption(c));
            if (!part.empty()) text += (text.empty() ? "" : " ") + part;
        }
    return text;
}

// The tutorial arrow, a child of the main window; its rectangle when shown.
bool ArrowRect(RECT* out) {
    for (Ptr c : game::Children(game::MainWindow()))
        if (game::IsKindOf(c, "cTSWinTutorialHighlight") && game::Visible(c)) {
            *out = game::Rect(c);
            return true;
        }
    return false;
}

// The tip icon exists while a tip waits; it blinks, so visibility is not asked.
Ptr TipIcon(Ptr lotView) {
    for (Ptr c : game::Children(lotView))
        if (game::IsKindOf(c, "cTutorialIcon")) return c;
    return 0;
}

bool TipIconShown(Ptr lotView) { return TipIcon(lotView) != 0; }

bool SameRect(const RECT& a, const RECT& b) {
    return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}

// "la flecha señala a Ver de cerca +, en controles"; over the lot, the object under the arrow's
// tip ("la flecha señala a Cubo de basura, exterior, a 5 casillas al sur") or the bare spot.
std::string ArrowLine(const RECT& arrow) {
    std::string group;
    std::string item = NearestItemTo(arrow, &group);
    if (!item.empty()) {
        if (group.empty()) return util::Format(S(Str::TutorialPointsAt), item.c_str());
        return util::Format(S(Str::TutorialPointsAtIn), item.c_str(), group.c_str());
    }
    int viewX, viewY;
    if (!game::ScreenPointToView((arrow.left + arrow.right) / 2, arrow.bottom + 6, &viewX, &viewY)) return "";
    game::LotObject sim;
    int level = game::GetObject(game::SelectedSim(), &sim) ? sim.level : 1;
    int tileX, tileY;
    game::Ptr object = game::ObjectAtView(viewX, viewY, level, &tileX, &tileY);
    std::string what = object ? world::DescribeObject(object) : world::DescribeSpot(tileX, tileY, level);
    if (what.empty()) return "";
    return util::Format(S(object ? Str::TutorialPointsAt : Str::TutorialPointsAtSpot), what.c_str());
}

}  // namespace

void Reset() { g = State{}; }

void Tick() {
    if (!LotViewActive()) {
        if (!g.lastText.empty() || g.arrowShown || g.tipShown) Reset();
        return;
    }
    Ptr lotView = game::InputRoot();
    Ptr panel = StepPanel(lotView);
    std::string text = panel ? StepText(panel) : "";
    RECT arrow = {};
    bool arrowShown = ArrowRect(&arrow);
    bool newStep = !text.empty() && text != g.lastText;
    bool arrowMoved = arrowShown && (!g.arrowShown || !SameRect(arrow, g.lastArrow));
    if (newStep) {
        g.lastText = text;
        int index = StepIndex(text);
        Log("tutorial step %d: %s", index, text.c_str());
        const KeyboardStep* step = KeyboardStepAt(index);
        buffers::Event(std::string(S(Str::TutorialSays)) + " " + (step ? S(step->text) : text.c_str()));
        if (arrowShown) {
            std::string line = ArrowLine(arrow);
            if (!line.empty()) buffers::Event(line);
        }
        if (!step)
            if (const Hint* h = HintAt(index)) buffers::Event(S(h->hint));
    } else if (arrowMoved) {  // a help tip's arrow points at something on the lot
        std::string line = ArrowLine(arrow);
        if (!line.empty()) buffers::Event(line);
    }
    if (text.empty()) g.lastText.clear();  // the same step shown again later is read again
    g.arrowShown = arrowShown;
    g.lastArrow = arrow;
    bool tip = game::HelpTipPending();
    if (tip && !g.tipShown) buffers::Event(S(Str::TipAvailable));
    g.tipShown = tip;
    bool running = game::TutorialObjectPresent();
    if (g.running && !running) buffers::Event(S(Str::TutorialOver));
    g.running = running;
}

bool Running() { return g.running; }

bool GuardEscape() {
    if (!g.running || g.lastText.empty()) return false;  // only while a step is shown
    ULONGLONG now = GetTickCount64();
    if (g.escapeAt && now - g.escapeAt < 4000) {
        g.escapeAt = 0;
        return false;
    }
    g.escapeAt = now;
    speech::Say(S(Str::TutorialEscapeAgain));
    return true;
}

void OpenTip() {
    Ptr icon = LotViewActive() ? TipIcon(game::InputRoot()) : 0;
    if (!icon || !game::HelpTipPending()) {
        speech::Say(S(Str::NoTip));
        return;
    }
    Log("help tip: showing it");
    if (!game::ShowHelpTip()) game::Press(icon);  // the icon's own click path as a fallback
}

bool IsStepPanel(Ptr widget) {
    return game::IsKindOf(widget, "cTSObjectDialog") && game::IsKindOf(game::Parent(widget), "cDDDSimsView");
}

bool IsStepGroup(const Group& g) { return IsStepPanel(g.key); }

bool IsStepHideButton(const Item& it) {
    return it.role == Role::Button && it.label.empty() && IsStepPanel(game::Parent(it.widget));
}

}  // namespace nav::tutorial
