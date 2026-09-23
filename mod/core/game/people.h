// Sims and families: names and biographies by neighbor ID, the family picker's rows, need meters.
#pragma once

#include <string>
#include <vector>

#include "memory.h"

namespace game {

// Every Sim in the neighborhood has a neighbor ID. UTF-8; empty when unknown.
std::string NeighborName(int id);
std::string NeighborBio(int id);

// Family picker (cWinPickFamily): one cPickFamilyItem row per family.
std::string FamilyRowName(Ptr row);
std::vector<int> FamilyRowMemberIds(Ptr row);  // neighbor IDs of the visible portraits
std::vector<Ptr> FamilyRowMembers(Ptr row);    // the visible portraits (cPickFamilyItemMember)
int FamilyMemberId(Ptr portrait);              // neighbor ID, -1 if none

// A Sim's queued actions by name, the current one first ("Tomar un tentempié"). Empty when the
// Sim has none or the game's queue storage is not set up yet (before the first lot view).
std::vector<std::string> SimActions(int neighborId);

// The same queue with each entry's state: running = the interaction the Sim is doing now (the
// others wait their turn).
struct SimAction {
    std::string name;
    bool running = false;
    bool byPlayer = false;  // queued by the player (not the Sim's free will)
};
std::vector<SimAction> SimActionList(int neighborId);

// A Sim's needs, -100 (empty) .. 100 (full), by motive number (Behavior.iff STR# 134).
enum class Need { Energy = 5, Comfort = 6, Hunger = 7, Hygiene = 8, Bladder = 9, Room = 13, Social = 14, Fun = 15 };
bool SimNeed(int neighborId, Need need, int* value);

// A Sim's mood, -100 (miserable) .. 100 (elated); false when unknown.
bool SimMood(int neighborId, int* mood);
// The family panel's mood meter: -5 (five red bars) .. 5 (five green bars).
int MoodBars(int mood);

// Skills as the job subpanel shows them: slot 0..5 = cooking, mechanical, charisma, body, logic,
// creativity. Points 0..1000; the panel draws one bar per 100.
bool SkillPoints(int neighborId, int slot, int* points);
std::vector<Ptr> JobPanelSkillButtons(Ptr jobPanel);  // index = slot
// Job, pay, performance, family friends needed for the next promotion. Their captions hold the
// values ("Decano de universidad", "$700", "Excelente", "Necesarios 9") once the panel is drawn.
std::vector<Ptr> JobPanelInfoButtons(Ptr jobPanel);

// A Sim's personality, 0..10 per trait, in the game's order: neat, outgoing, active, playful,
// nice (person data 7, 6, 3, 5, 2).
constexpr int kTraitCount = 5;
bool Personality(int neighborId, int out[kTraitCount]);

// A personality trait widget (cWinPersonality): the trait's points 0..10 for the Sim it shows.
bool TraitPoints(Ptr trait, int* points);

// An interest widget (cWinInterest): points 0..10.
int InterestPoints(Ptr interest);

// Live mode family panel (cWinPeople): the portrait buttons, visible or not.
std::vector<Ptr> FamilyMemberButtons(Ptr peoplePanel);
Ptr NeedsBox(Ptr peoplePanel);
// The subpanel toggles in the game's order: Estado de ánimo, Relaciones, Empleo, Casa,
// Personalidad, Interés, Inventario.
std::vector<Ptr> SubpanelButtons(Ptr peoplePanel);
// One of the two halves of the mood meter (plain widgets titled "Grado de humor" on the panel).
bool IsMoodMeter(Ptr w);

// A card on the relationships subpanel (cWinRelationship); the tooltip names the Sim.
struct Relationship {
    int daily = 0;     // -100..100
    int lifetime = 0;  // -100..100
};
Relationship ReadRelationship(Ptr card);

// Need meters (cWinMotive); the caption is the need's name.
int MotiveValue(Ptr motive);          // -100 (empty) .. 100 (full)
std::string MotiveHelp(Ptr motive);   // the game's explanation of the need, UTF-8

}  // namespace game
