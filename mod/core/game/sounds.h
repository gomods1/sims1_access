// The game's named sound events: every object and UI sound goes through one function
// (0x527250, "play sound event by name" with the object that makes it), so the mod hears which
// sound is played and by whom. A blind player hears the sounds already (the phone, the doorbell,
// alarms): only the ones whose meaning the sound does not give are kept, which car is honking
// (every car honks alike). The rest is ignored in the hook itself.
#pragma once

#include <string>
#include <vector>

namespace game {

enum class SoundAlert {
    Carpool,    // a carpool honking for a Sim going to work
    SchoolBus,
    PoliceCar,
};

struct SoundEvent {
    SoundAlert alert;
    std::string name;  // the game's event name ("phone_ring")
    int source = 0;    // object id of the object that plays it (<= 0: the interface)
};

// Called by the hook for every sound event: keeps the alerts.
void OnSoundEvent(const char* name, int source);
// The alerts played since the last call, oldest first.
std::vector<SoundEvent> TakeSoundEvents();

}  // namespace game
