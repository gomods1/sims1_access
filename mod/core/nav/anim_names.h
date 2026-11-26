// What the game's animations mean, in the player's language: the descriptions in
// Sims2Access\anims\<tag>.tsv (key TAB text, written for both Sims mods; tools/anim_keys.py,
// tools/anim_style.md), looked up by the animation's key.
#pragma once

#include <string>
#include <vector>

namespace nav::anims {

void Init(const wchar_t* modDir);
// An animation's key, as tools/anim_keys.py makes it (change both together): its internal name
// without who does it, in lowercase words, loop marks, numbers, variants and rig marks left out:
// "a2o-bed-relax-loop-loop2_anim" -> "bed relax".
std::string Key(const std::string& raw);
// What to say for an animation: its description; else its key's words; empty when it says
// nothing (blinking, a neutral face).
std::string Spoken(const std::string& raw);
// The descriptions of a Sim's running animations as one line, a clause already said left out
// ("calming down, waiting" and "hunched, waiting" give "calming down, waiting, hunched").
std::string Join(const std::vector<std::string>& descriptions);

}  // namespace nav::anims
