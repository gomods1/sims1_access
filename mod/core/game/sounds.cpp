#include "sounds.h"

#include <cstring>
#include <set>

#include "../util/log.h"

namespace game {
namespace {

// Event names from the objects' FWAV resources (every IFF of the install was listed). A name
// matches an entry that equals it, or that it starts with when the entry ends with '*'.
struct Pattern {
    const char* name;
    SoundAlert alert;
};
const Pattern kPatterns[] = {
    {"car_schoolbus_honk*", SoundAlert::SchoolBus},
    {"car_squad_honk*", SoundAlert::PoliceCar},
    {"car_*_honk*", SoundAlert::Carpool},
};

bool Matches(const char* pattern, const char* name) {
    // '*' matches any run of characters.
    if (!*pattern) return !*name;
    if (*pattern == '*') {
        for (const char* n = name;; n++) {
            if (Matches(pattern + 1, n)) return true;
            if (!*n) return false;
        }
    }
    return *name == *pattern && Matches(pattern + 1, name + 1);
}

std::vector<SoundEvent> g_events;
std::set<std::string> g_seen;  // development: every distinct name is logged once

}  // namespace

void OnSoundEvent(const char* name, int source) {
    if (!name || !*name) return;
    if (util::VerboseLogging() && g_seen.size() < 2000 && g_seen.insert(name).second)
        LogVerbose("sound event '%s' (object %d)", name, source);
    for (const Pattern& p : kPatterns)
        if (Matches(p.name, name)) {
            if (g_events.size() < 64) g_events.push_back({p.alert, name, source});
            return;
        }
}

std::vector<SoundEvent> TakeSoundEvents() {
    std::vector<SoundEvent> out;
    out.swap(g_events);
    return out;
}

}  // namespace game
