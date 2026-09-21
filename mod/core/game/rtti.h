// Class identity of game objects, read at runtime from MSVC RTTI (the CompleteObjectLocator
// behind each vtable). Robust against pointers that are not objects: those have no class.
#pragma once

#include <string>

#include "memory.h"

namespace game {

std::string ClassName(Ptr obj);           // "cTSWinBtn"; empty when not a known object
bool IsKindOf(Ptr obj, const char* cls);  // obj's class is cls or derives from it
void ClearRttiCache();

}  // namespace game
