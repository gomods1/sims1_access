// The create-a-Sim screen (cWinDesignCharacter): personality bars and the points left to spend.
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

// A personality bar (cWinPersonalityWidget): the trait's name and its points 0..10.
bool IsTraitBar(Ptr w);
std::string TraitBarName(Ptr bar);  // UTF-8
int TraitBarPoints(Ptr bar);
int PersonalityPointsLeft(Ptr bar);  // of the 25 a new Sim gets; -1 unknown

// One point up or down, the way a click beside or on the filled bar does it (the game checks
// the points left and plays its sounds). false when the click could not be delivered.
bool AdjustTraitBar(Ptr bar, int delta);

// The head or body shown in the preview: its place in the list and its mesh name
// ("c036malgt_dave", "b011mafitlgt_beachguy2"). Names follow a pattern: letter + number, sex and
// age (ma, fa, mc, fc), for bodies a shape (fit, fat, skn), skin (lgt, med, drk), then a word.
enum class LookPart { Head, Body };
struct Look {
    int index = 0;  // 0-based
    int count = 0;
    std::string mesh;
};
bool CurrentLook(Ptr screen, LookPart part, Look* out);
// The appearance controls in reading order: child, adult, male, female, light, medium, dark
// skin, the preview, previous/next head, previous/next body.
std::vector<Ptr> LookControls(Ptr screen);
Ptr ZodiacButton(Ptr screen);

// The wardrobe's clothing choice (cWinClothingChoiceDlg): the outfit shown, as a body look.
bool CurrentOutfit(Ptr dialog, Look* out);
Ptr OutfitPreview(Ptr dialog);
bool IsOutfitArrow(Ptr dialog, Ptr button);
// A pet pen's adoption picker (cWinPetSelectDlg): the same preview, over the pen's breeds.
bool IsPetDialog(Ptr dialog);
// Makin' Magic's transformation (cWinTransformMeDlg): the look its arrows show, 0-based; -1 when
// the button is not one of its arrows.
int TransformChoice(Ptr dialog, Ptr pressed);

// Which arrow a button is: the part it changes, or false for any other widget.
bool IsLookArrow(Ptr screen, Ptr button, LookPart* part);

}  // namespace game
