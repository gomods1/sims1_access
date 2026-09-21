// Fault-tolerant access to the game's memory and code. A bad pointer or a crash inside a game
// function yields a failure value and a log line, never a crash of the game.
#pragma once

#include <windows.h>

#include <cstdint>
#include <string>

namespace game {

using Ptr = uintptr_t;

// Resolves the module base, checks the prologue of every game function the mod calls or hooks.
void InitMemory(HMODULE exe);

Ptr Va(Ptr preferredVa);   // preferred-base VA -> runtime address
bool InImage(Ptr p);        // inside Sims.exe

bool Read(Ptr addr, void* out, size_t n);
template <class T>
T Rd(Ptr addr, T fallback = T{}) {
    T v;
    return Read(addr, &v, sizeof v) ? v : fallback;
}
bool WriteU8(Ptr addr, uint8_t value);
bool WriteU32(Ptr addr, uint32_t value);

std::string ReadStdString(Ptr addr);          // MSVC std::string, raw cp1252 bytes
std::string ReadStdStringUtf8(Ptr addr);      // same, converted to UTF-8

// True when the function at this preferred VA matched its expected prologue at startup.
bool FunctionOk(Ptr preferredVa);

// Runs fn under SEH. fn must be a thin wrapper around a call into the game (no objects with
// destructors live across the call).
template <class Fn>
bool Guarded(Fn&& fn) {
    __try {
        fn();
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

}  // namespace game
