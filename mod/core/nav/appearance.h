// Spoken descriptions of create-a-Sim's heads and bodies (and the wardrobe's outfits): the game
// has no text for them, so the mod keeps its own, written from the previews.
#pragma once

#include <string>

namespace nav::appearance {

// "pelo negro, corto, perilla" for a head or "camisa, vaqueros, azul" for a body, by its mesh name
// ("c036malgt_dave"); empty for meshes the mod has no description of (custom content).
std::string Describe(const std::string& mesh);

}  // namespace nav::appearance
