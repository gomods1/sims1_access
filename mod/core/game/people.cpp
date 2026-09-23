#include "people.h"

#include <cstring>

#include <cmath>
#include <unordered_map>

#include "addresses.h"
#include "rtti.h"
#include "ui.h"

namespace game {

std::string NeighborName(int id) {
    typedef void(__cdecl* NameFn)(int id, void* out);
    static std::unordered_map<int, std::string> cache;
    if (id < 0 || !FunctionOk(addr::kNeighborName)) return "";
    auto hit = cache.find(id);
    if (hit != cache.end()) return hit->second;
    // A raw MSVC std::string in its empty state (size 0, capacity 15). A name longer than 15
    // bytes goes to the game's heap and is leaked on purpose: names are cached, so once per Sim.
    uint32_t str[6] = {0, 0, 0, 0, 0, 15};
    auto fn = (NameFn)Va(addr::kNeighborName);
    if (!Guarded([&] { fn(id, str); })) return "";
    std::string name = ReadStdStringUtf8((Ptr)str);
    if (!name.empty()) cache[id] = name;
    return name;
}

std::string NeighborBio(int id) {
    typedef void*(__cdecl* BioFn)(int id);
    if (id < 0 || !FunctionOk(addr::kNeighborBio)) return "";
    Ptr str = 0;
    auto fn = (BioFn)Va(addr::kNeighborBio);
    if (!Guarded([&] { str = (Ptr)fn(id); }) || !str) return "";
    return ReadStdStringUtf8(str);
}

std::string FamilyRowName(Ptr row) { return ReadStdStringUtf8(row + addr::famrow::kName); }

std::vector<Ptr> FamilyRowMembers(Ptr row) {
    std::vector<Ptr> members;
    for (int i = 0; i < addr::famrow::kMemberSlots; i++) {
        Ptr member = Rd<Ptr>(row + addr::famrow::kMembers + i * 4);
        if (IsKindOf(member, "cPickFamilyItemMember") && Visible(member)) members.push_back(member);
    }
    return members;
}

int FamilyMemberId(Ptr portrait) { return Rd<int>(portrait + addr::fammember::kNeighborId, -1); }

std::vector<int> FamilyRowMemberIds(Ptr row) {
    std::vector<int> ids;
    for (int i = 0; i < addr::famrow::kMemberSlots; i++) {
        Ptr member = Rd<Ptr>(row + addr::famrow::kMembers + i * 4);
        if (IsKindOf(member, "cPickFamilyItemMember") && Visible(member))
            ids.push_back(Rd<int>(member + addr::fammember::kNeighborId, -1));
    }
    return ids;
}

// Borrows the queue icons' scratch vector: emptied with the game's entry destructor, filled for
// this Sim, read, emptied again (the icons refill it for the selected Sim on their next update).
std::vector<SimAction> SimActionList(int neighborId) {
    typedef void(__cdecl * QueueFn)(int id, void* out);
    typedef void(__thiscall * DtorFn)(void* entry);
    std::vector<SimAction> actions;
    if (neighborId < 0 || !FunctionOk(addr::kSimQueue) || !FunctionOk(addr::kQueueEntryDtor)) return actions;
    Ptr vec = Va(addr::kQueueScratch);
    Ptr first = Rd<Ptr>(vec), last = Rd<Ptr>(vec + 4), end = Rd<Ptr>(vec + 8);
    if (!first || !end || last < first || end < last || (last - first) % addr::kQueueEntrySize) return actions;
    auto empty = [&] {
        Ptr begin = Rd<Ptr>(vec), stop = Rd<Ptr>(vec + 4);
        for (Ptr e = begin; e < stop; e += addr::kQueueEntrySize) ((DtorFn)Va(addr::kQueueEntryDtor))((void*)e);
        WriteU32(vec + 4, (uint32_t)begin);
    };
    Guarded([&] {
        empty();
        ((QueueFn)Va(addr::kSimQueue))(neighborId, (void*)vec);
        for (Ptr e = Rd<Ptr>(vec); e < Rd<Ptr>(vec + 4); e += addr::kQueueEntrySize) {
            SimAction a;
            a.name = ReadStdStringUtf8(e + addr::kQueueEntryName);
            a.running = Rd<uint8_t>(e + addr::kQueueEntryRunning) != 0;
            Ptr record = Rd<Ptr>(e + addr::kQueueEntryRecord);
            a.byPlayer = record && Rd<int>(record + addr::kQueueRecordPriority) >= addr::kPlayerPriority;
            if (!a.name.empty()) actions.push_back(a);
        }
        empty();
    });
    return actions;
}

std::vector<std::string> SimActions(int neighborId) {
    std::vector<std::string> names;
    for (const SimAction& a : SimActionList(neighborId)) names.push_back(a.name);
    return names;
}

namespace {

Ptr SimStateOf(int neighborId) {
    typedef void*(__cdecl * StateFn)(int id);
    if (neighborId < 0 || !FunctionOk(addr::kSimState)) return 0;
    Ptr state = 0;
    auto fn = (StateFn)Va(addr::kSimState);
    if (!Guarded([&] { state = (Ptr)fn(neighborId); })) return 0;
    return state;
}

}  // namespace

bool SimNeed(int neighborId, Need need, int* value) {
    Ptr state = SimStateOf(neighborId);
    int index = (int)need;
    if (!state || index < 0 || index >= addr::kMotiveCount) return false;
    float v = 0;
    if (!Read(state + addr::kSimStateMotives + index * 4, &v, sizeof v) || !std::isfinite(v)) return false;
    *value = (int)std::lround(v);
    return true;
}

bool SimMood(int neighborId, int* mood) {
    typedef void*(__cdecl * StateFn)(int id);
    if (neighborId < 0 || !FunctionOk(addr::kSimState)) return false;
    Ptr state = 0;
    auto fn = (StateFn)Va(addr::kSimState);
    if (!Guarded([&] { state = (Ptr)fn(neighborId); }) || !state) return false;
    float value = 0;
    if (!Read(state + addr::kSimStateMood, &value, sizeof value) || !std::isfinite(value)) return false;
    *mood = (int)std::lround(value);
    return true;
}

// The meter's own rounding (0x4bb980): (mood + 100) * 11 / 201 - 5, clamped to five bars.
int MoodBars(int mood) {
    int bars = (int)std::lround((mood + 100.0f) * 11.0f / 201.0f - 5.0f);
    return bars < -5 ? -5 : bars > 5 ? 5 : bars;
}

bool SkillPoints(int neighborId, int slot, int* points) {
    typedef int(__cdecl * SkillFn)(int id, int index);
    bool pet = slot >= kPetSkillSlot;
    int i = pet ? slot - kPetSkillSlot : slot;
    if (neighborId < 0 || i < 0 || i >= (pet ? addr::kPetPanelSkillCount : addr::kJobPanelSkillCount) ||
        !FunctionOk(addr::kSimSkill))
        return false;
    int index = Rd<int>(Va(pet ? addr::kPetPanelSkillIds : addr::kJobPanelSkillIds) + i * 4, -1);
    if (index < 0) return false;
    auto fn = (SkillFn)Va(addr::kSimSkill);
    return Guarded([&] { *points = fn(neighborId, index); });
}

namespace {

bool PersonDataValue(int id, int index, short* value) {
    typedef short*(__cdecl * DataFn)(int id);
    if (index < 0 || index > 200 || id < 0 || !FunctionOk(addr::kSimPersonData)) return false;
    Ptr data = 0;
    auto fn = (DataFn)Va(addr::kSimPersonData);
    if (!Guarded([&] { data = (Ptr)fn(id); }) || !data) return false;
    return Read(data + index * 2, value, sizeof *value);
}

}  // namespace

bool Personality(int neighborId, int out[kTraitCount]) {
    const int kIndices[kTraitCount] = {7, 6, 3, 5, 2};
    for (int i = 0; i < kTraitCount; i++) {
        short value = 0;
        if (!PersonDataValue(neighborId, kIndices[i], &value)) return false;
        out[i] = value / 100;
    }
    return true;
}

// What the widget draws (0x4bdd80): person data[index] / 100.
bool TraitPoints(Ptr trait, int* points) {
    short value = 0;
    if (!PersonDataValue(Rd<int>(trait + addr::trait::kNeighborId, -1), Rd<int>(trait + addr::trait::kIndex, -1), &value))
        return false;
    *points = value / 100;
    return true;
}

// As the game picks the popup (0x4be260): a Sim on the lot that is not a pet reads the Sims'
// texts, anyone else the pets'.
bool TraitHelp(Ptr trait, std::string* title, std::string* text) {
    typedef Ptr(__cdecl * PersonFn)(int id);
    int points = 0;
    if (!IsKindOf(trait, "cWinPersonality") || !TraitPoints(trait, &points)) return false;
    int level = points < 3 ? 0 : points >= 8 ? 2 : 1;
    Ptr person = 0;
    int id = Rd<int>(trait + addr::trait::kNeighborId, -1);
    if (FunctionOk(addr::kPersonByNeighbor)) Guarded([&] { person = ((PersonFn)Va(addr::kPersonByNeighbor))(id); });
    bool sim = person && Rd<short>(person + addr::kPersonPetFlags) <= 1;
    int at = level * addr::trait::kTextStride;
    *title = ReadStdStringUtf8(trait + (sim ? addr::trait::kTitles : addr::trait::kPetTitles) + at);
    *text = ReadStdStringUtf8(trait + (sim ? addr::trait::kTexts : addr::trait::kPetTexts) + at);
    return !title->empty() || !text->empty();
}

int InterestPoints(Ptr interest) { return Rd<int>(interest + addr::interest::kPoints); }
std::string InventoryItemName(Ptr gift) {
    if (!IsKindOf(gift, "cWinGift")) return "";
    std::string name = ReadStdStringUtf8(gift + addr::gift::kName);
    return name.empty() ? Tooltip(gift) : name;
}
std::string InventoryItemDescription(Ptr gift) {
    return IsKindOf(gift, "cWinGift") ? ReadStdStringUtf8(gift + addr::gift::kDescription) : "";
}
int InterestNumber(Ptr interest) { return Rd<int>(interest + addr::interest::kIndex, -1); }

std::vector<Ptr> JobPanelSkillButtons(Ptr jobPanel) {
    std::vector<Ptr> buttons;
    if (!IsKindOf(jobPanel, "cWinSubpanelJob")) return buttons;
    Ptr array = Rd<Ptr>(jobPanel + addr::jobpanel::kSkillButtons);
    for (int i = 0; array && i < addr::kJobPanelSkillCount; i++) buttons.push_back(Rd<Ptr>(array + i * 4));
    return buttons;
}

std::vector<Ptr> PetSkillButtons(Ptr petPanel) {
    std::vector<Ptr> buttons;
    if (!IsKindOf(petPanel, "cWinSubpanelDogSkills") && !IsKindOf(petPanel, "cWinSubpanelCatSkills")) return buttons;
    Ptr begin = Rd<Ptr>(petPanel + addr::kPetPanelSkillButtons), end = Rd<Ptr>(petPanel + addr::kPetPanelSkillButtons + 4);
    for (Ptr p = begin; p && p < end && buttons.size() < (size_t)addr::kPetPanelSkillCount; p += 4)
        buttons.push_back(Rd<Ptr>(p));
    return buttons;
}

std::vector<Ptr> JobPanelInfoButtons(Ptr jobPanel) {
    std::vector<Ptr> buttons;
    if (!IsKindOf(jobPanel, "cWinSubpanelJob")) return buttons;
    Ptr array = Rd<Ptr>(jobPanel + addr::jobpanel::kInfoButtons);
    for (int i = 0; array && i < 4; i++) buttons.push_back(Rd<Ptr>(array + i * 4));
    return buttons;
}

Ptr NeedsBox(Ptr peoplePanel) {
    if (!IsKindOf(peoplePanel, "cWinPeople")) return 0;
    return Rd<Ptr>(peoplePanel + addr::people::kNeedsBox);
}

std::vector<Ptr> SubpanelButtons(Ptr peoplePanel) {
    std::vector<Ptr> buttons;
    if (!IsKindOf(peoplePanel, "cWinPeople")) return buttons;
    for (int i = 0; i < addr::people::kSubpanelCount; i++) {
        Ptr b = Rd<Ptr>(peoplePanel + addr::people::kSubpanelButtons + i * 4);
        if (IsKindOf(b, "cTSWinBtn")) buttons.push_back(b);
    }
    return buttons;
}

bool IsMoodMeter(Ptr w) {
    return ClassName(w) == "cTSWin" && IsKindOf(Parent(w), "cWinPeople") && !Tooltip(w).empty();
}

Relationship ReadRelationship(Ptr card) {
    Relationship r;
    r.daily = Rd<int>(card + addr::relcard::kDaily);
    r.lifetime = Rd<int>(card + addr::relcard::kLifetime);
    return r;
}

bool SimRelationship(int fromId, int toId, Relationship* out) {
    typedef void(__cdecl * RelFn)(int from, int to, void* out);
    if (fromId < 0 || toId < 0 || fromId == toId || !FunctionOk(addr::kSimRelationship)) return false;
    alignas(4) unsigned char buf[addr::kRelOutSize] = {};
    auto fn = (RelFn)Va(addr::kSimRelationship);
    if (!Guarded([&] { fn(fromId, toId, buf); })) return false;
    memcpy(&out->daily, buf + addr::kRelOutDaily, 4);
    memcpy(&out->lifetime, buf + addr::kRelOutLifetime, 4);
    return true;
}

std::vector<Ptr> FamilyMemberButtons(Ptr peoplePanel) {
    std::vector<Ptr> buttons;
    if (!IsKindOf(peoplePanel, "cWinPeople")) return buttons;
    Ptr begin = Rd<Ptr>(peoplePanel + addr::people::kMemberButtons);
    Ptr end = Rd<Ptr>(peoplePanel + addr::people::kMemberButtons + 4);
    if (!begin || end < begin || end - begin > 64 * 4) return buttons;
    for (Ptr p = begin; p < end; p += 4) {
        Ptr b = Rd<Ptr>(p);
        if (IsKindOf(b, "cTSWinBtn")) buttons.push_back(b);
    }
    return buttons;
}

int MotiveValue(Ptr motive) { return Rd<int>(motive + addr::motive::kValue); }

std::string MotiveHelp(Ptr motive) { return ReadStdStringUtf8(motive + addr::motive::kHelp); }

}  // namespace game
