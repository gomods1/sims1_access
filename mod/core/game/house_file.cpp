#include "house_file.h"

#include <windows.h>
#include <knownfolders.h>
#include <shlobj.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <unordered_map>

#include "../util/game_settings.h"
#include "../util/log.h"
#include "addresses.h"
#include "lot.h"

namespace game::housefile {
namespace {

constexpr size_t kIffHeader = 64, kChunkHeader = 76;
constexpr uint16_t kPetFunctionFlag = 0x40;
constexpr char kIffMagic[] = "IFF FILE 2.5:TYPE FOLLOWED BY SIZE";

uint32_t Be32(const std::string& d, size_t at) {
    return (uint32_t)(uint8_t)d[at] << 24 | (uint32_t)(uint8_t)d[at + 1] << 16 | (uint32_t)(uint8_t)d[at + 2] << 8 |
           (uint8_t)d[at + 3];
}
uint16_t Be16(const std::string& d, size_t at) { return (uint16_t)((uint8_t)d[at] << 8 | (uint8_t)d[at + 1]); }
uint32_t Le32(const std::string& d, size_t at) {
    return at + 4 > d.size() ? 0
                             : (uint32_t)(uint8_t)d[at] | (uint32_t)(uint8_t)d[at + 1] << 8 |
                                   (uint32_t)(uint8_t)d[at + 2] << 16 | (uint32_t)(uint8_t)d[at + 3] << 24;
}
uint16_t Le16(const std::string& d, size_t at) {
    return at + 2 > d.size() ? 0 : (uint16_t)((uint8_t)d[at] | (uint8_t)d[at + 1] << 8);
}

// The current neighborhood's folder ("...\UserData\"), from the setting the game keeps current.
// The game saves under the user's Saved Games folder (0x454a80: SHGetKnownFolderPath with
// FOLDERID_SavedGames, then "\Electronic Arts\The Sims 25\"); the install folder's UserData is
// only the first copy and never changes. It is the fallback when no save folder exists.
std::string NeighborhoodDir() {
    std::string hood = util::ReadGameSetting("SIMS_CURRENT_NEIGHBORHOOD_PATH");
    if (hood.empty()) hood = "UserData";
    PWSTR saved = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SavedGames, KF_FLAG_DONT_VERIFY, nullptr, &saved)) && saved) {
        char path[MAX_PATH * 2] = {};
        WideCharToMultiByte(CP_ACP, 0, saved, -1, path, sizeof path, nullptr, nullptr);
        CoTaskMemFree(saved);
        std::string dir = std::string(path) + "\\Electronic Arts\\The Sims 25\\" + hood + "\\";
        if (GetFileAttributesA((dir + "Neighborhood.iff").c_str()) != INVALID_FILE_ATTRIBUTES) return dir;
    }
    char exe[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string dir = exe;
    size_t slash = dir.find_last_of("\\/");
    dir = slash == std::string::npos ? std::string() : dir.substr(0, slash + 1);
    return dir + hood + "\\";
}

bool ReadFile(const std::string& path, std::string* data) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    data->assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

// Chunks by (type, id): their data after the chunk header.
using Chunks = std::map<std::pair<std::string, int>, std::string>;
bool ReadChunks(const std::string& data, Chunks* out) {
    if (data.size() < kIffHeader || data.compare(0, sizeof kIffMagic - 1, kIffMagic) != 0) return false;
    for (size_t pos = kIffHeader; pos + kChunkHeader <= data.size();) {
        uint32_t size = Be32(data, pos + 4);
        if (size < kChunkHeader || pos + size > data.size()) break;
        (*out)[{data.substr(pos, 4), Be16(data, pos + 8)}] = data.substr(pos + kChunkHeader, size - kChunkHeader);
        pos += size;
    }
    return true;
}

// Arry: u32 0, u32 width, u32 height, u32 bytes per tile, u32 1, then runs over the bytes: u16
// with bit 15 set = that many copies of the next byte (padded to 2), else that many bytes as they
// are (padded to an even count).
bool ReadArray(const std::string& d, int* width, std::vector<uint8_t>* out) {
    if (d.size() < 20) return false;
    int w = (int)Le32(d, 4), h = (int)Le32(d, 8), per = (int)Le32(d, 12);
    if (w <= 0 || w > 256 || h != w || per <= 0 || per > 16) return false;
    out->assign((size_t)w * h * per, 0);
    size_t pos = 20, at = 0;
    while (pos + 2 <= d.size() && at < out->size()) {
        uint16_t run = Le16(d, pos);
        pos += 2;
        size_t count = run & 0x7fff;
        if (count == 0) break;
        count = (std::min)(count, out->size() - at);
        if (run & 0x8000) {
            if (pos >= d.size()) return false;
            memset(out->data() + at, (uint8_t)d[pos], count);
            pos += 2;
        } else {
            if (pos + count > d.size()) return false;
            memcpy(out->data() + at, d.data() + pos, count);
            pos += count + (count & 1);
        }
        at += count;
    }
    *width = w;
    return true;
}

// objt: u32 0, u32 version, "tjbo", then per type: u32 GUID (0 = an empty slot), u16 x4, u16 type
// id (what ObjM refers to), u16 OBJD object type, NUL-terminated name padded to even; version 3
// adds a u32 after each entry.
struct TypeEntry {
    uint32_t guid = 0;
    int kind = 0;
};
std::map<int, TypeEntry> ReadTypes(const std::string& d) {
    std::map<int, TypeEntry> out;
    uint32_t version = Le32(d, 4);
    size_t pos = 12;
    while (pos + 16 <= d.size()) {
        uint32_t guid = Le32(d, pos);
        pos += 4;
        if (!guid) continue;
        int id = Le16(d, pos + 8), kind = Le16(d, pos + 10);
        pos += 12;
        size_t end = d.find('\0', pos);
        if (end == std::string::npos) break;
        size_t length = end - pos;
        pos = end + 1 + ((length + 1) & 1);
        if (version >= 3) pos += 4;
        out[id] = {guid, kind};
    }
    return out;
}

// ObjM (FreeSO's OBJM.cs): u32 0, u32 version, "MjbO", u8 1 (field encoding), then fields
// packed in bits (most significant first): 0 = the value 0; else 2 bits pick the width (5, 8, 13
// or 16 bits for a short; 6, 11, 21 or 32 for an int) and the value follows, sign-extended. It
// starts with pairs (object id, objt type id) up to an id of 0; from the next whole byte, one
// block per object: a plain u32 offset (from just after "MjbO") of the next block, then its own
// fields: footprint (4 ints), y and x (ints, 1/16 tile: a tile's centre is tile * 16 + 8; -16
// when not placed), level (int), a short, the attribute count and the attributes (shorts), 8
// temporaries, then the 68 object data shorts (VMStackObjectVariable: 1 direction, 2 container,
// 11 object id). Verified on the Goths' lot: every placed object on the tile the live game has.
class Fields {
public:
    Fields(const std::string& d, size_t pos, size_t end) : d_(d), pos_(pos), end_((std::min)(end, d.size())) {}
    int Short() { return (int)Field(kShort); }
    int Int() { return (int)Field(kInt); }
    bool Done() const { return pos_ >= end_; }
    size_t NextByte() const { return pos_ + (bit_ ? 1 : 0); }

private:
    static constexpr int kShort[4] = {5, 8, 13, 16};
    static constexpr int kInt[4] = {6, 11, 21, 32};
    int64_t Field(const int (&widths)[4]) {
        if (!Bit()) return 0;
        int width = widths[Bits(2)];
        int64_t v = Bits(width);
        if (v & (int64_t(1) << (width - 1))) v -= int64_t(1) << width;
        return v;
    }
    int Bit() {
        if (pos_ >= end_) return 0;
        int b = ((uint8_t)d_[pos_] >> (7 - bit_)) & 1;
        if (++bit_ == 8) bit_ = 0, pos_++;
        return b;
    }
    uint32_t Bits(int n) {
        uint32_t v = 0;
        for (int i = 0; i < n; i++) v = v << 1 | Bit();
        return v;
    }
    const std::string& d_;
    size_t pos_, end_;
    int bit_ = 0;
};

struct Placed {
    int id = 0, typeId = 0;
    int x = -1, y = -1, level = 0, container = 0;
};

std::vector<Placed> ReadObjects(const std::string& d) {
    std::vector<Placed> out;
    constexpr size_t kBase = 12;  // offsets count from just after "MjbO"
    if (d.size() < 13 || d.compare(8, 4, "MjbO") != 0 || d[12] != 1) return out;
    Fields f(d, 13, d.size());
    while (!f.Done() && out.size() < 20000) {
        Placed p;
        p.id = f.Short();
        if (p.id <= 0) break;
        p.typeId = f.Short();
        out.push_back(p);
    }
    size_t pos = f.NextByte();
    for (Placed& p : out) {
        if (pos + 4 > d.size()) break;
        size_t end = kBase + Le32(d, pos);
        if (end <= pos + 4 || end > d.size()) break;
        Fields o(d, pos + 4, end);
        for (int i = 0; i < 4; i++) o.Int();  // footprint
        int y = o.Int(), x = o.Int();
        p.level = o.Int();
        o.Short();
        int attributes = o.Short();
        if (attributes < 0 || attributes > 1000) break;
        for (int i = 0; i < attributes + 8; i++) o.Short();
        int data[12] = {};
        for (int& v : data) v = o.Short();
        if (data[11] != p.id) {
            Log("house files: the block of object %d reads as object %d", p.id, data[11]);
            break;
        }
        if (x >= 0 && y >= 0) {
            p.x = x >> 4;
            p.y = y >> 4;
            p.container = data[2];
        }
        pos = end;
    }
    return out;
}

}  // namespace

uint8_t House::Walls(int level, int x, int y) const {
    if (level < 1 || level > kLevels || x < 0 || y < 0 || x >= size || y >= size) return 0;
    const std::vector<uint8_t>& w = floors[level - 1].walls;
    size_t at = ((size_t)y * size + x) * 8;
    return at < w.size() ? w[at] : 0;
}

uint8_t House::FloorAt(int level, int x, int y) const {
    if (level < 1 || level > kLevels || x < 0 || y < 0 || x >= size || y >= size) return 0;
    const std::vector<uint8_t>& f = floors[level - 1].floors;
    size_t at = (size_t)y * size + x;
    return at < f.size() ? f[at] : 0;
}

uint8_t House::FlagsAt(int level, int x, int y) const {
    if (level < 1 || level > kLevels || x < 0 || y < 0 || x >= size || y >= size) return 0;
    const std::vector<uint8_t>& f = floors[level - 1].flags;
    size_t at = (size_t)y * size + x;
    return at < f.size() ? f[at] : 0;
}

const Object* House::Find(int id) const {
    for (const Object& o : objects)
        if (o.id == id) return &o;
    return nullptr;
}

Ptr ObjectType(uint32_t guid) {
    // ObjectFolder's std::unordered_map<GUID, type>: its node list (next, prev, key, value) has
    // its sentinel at +kGuidList and the count after it (0x5738b0 looks types up in it).
    static Ptr cachedFolder = 0;
    static uint32_t cachedCount = 0;
    static std::unordered_map<uint32_t, Ptr> types;
    Ptr folder = Rd<Ptr>(Rd<Ptr>(Va(addr::kSimRoot)) + addr::folder::kObjectFolder);
    if (!folder) return 0;
    Ptr head = Rd<Ptr>(folder + addr::folder::kGuidList);
    uint32_t count = Rd<uint32_t>(folder + addr::folder::kGuidList + 4);
    if (folder != cachedFolder || count != cachedCount) {
        types.clear();
        cachedFolder = folder;
        cachedCount = count;
        if (head && count < 100000) {
            Ptr node = Rd<Ptr>(head);
            for (uint32_t i = 0; node && node != head && i <= count; i++, node = Rd<Ptr>(node))
                types[Rd<uint32_t>(node + 8)] = Rd<Ptr>(node + 12);
        }
        Log("house files: %u object types known", (unsigned)types.size());
    }
    auto hit = types.find(guid);
    return hit == types.end() ? 0 : hit->second;
}

bool Read(int number, House* out) {
    char name[32];
    snprintf(name, sizeof name, "Houses\\House%02d.iff", number);
    std::string path = NeighborhoodDir() + name, data;
    Chunks chunks;
    if (!ReadFile(path, &data) || !ReadChunks(data, &chunks)) {
        LogVerbose("house files: cannot read %s", path.c_str());
        return false;
    }
    House h;
    // Arry ids per floor: floors, walls, tile flags (the upper floor's add 100). Arry 3 / 103
    // keeps one object per tile and hides the rest (whether a window or the sofa under it shows
    // depends on the save), so objects are placed from their own blocks in ObjM.
    static const int kArrays[kLevels][3] = {{1, 2, 8}, {101, 102, 108}};
    for (int level = 0; level < kLevels; level++) {
        Floor& f = h.floors[level];
        int wf = 0, ww = 0, wg = 0;
        auto floors = chunks.find({"Arry", kArrays[level][0]});
        auto walls = chunks.find({"Arry", kArrays[level][1]});
        auto flags = chunks.find({"Arry", kArrays[level][2]});
        if (floors == chunks.end() || walls == chunks.end() || flags == chunks.end()) {
            if (level == 0) return false;
            continue;
        }
        if (!ReadArray(floors->second, &wf, &f.floors) || !ReadArray(walls->second, &ww, &f.walls) ||
            !ReadArray(flags->second, &wg, &f.flags) || wf != ww || wf != wg || f.walls.size() != (size_t)ww * ww * 8 ||
            f.flags.size() != (size_t)wg * wg) {
            Log("house files: House%02d floor %d not understood", number, level + 1);
            return false;
        }
        h.size = wf;
    }
    // The lot ends where the ground floor's tiles stop being floorable, seeded or outdoors (the
    // maps are larger than any lot).
    for (int y = 0; y < h.size; y++)
        for (int x = 0; x < h.size; x++)
            if (h.FlagsAt(1, x, y) & 0x07) h.lotSize = (std::max)(h.lotSize, (std::max)(x, y) + 1);
    auto objt = chunks.find({"objt", 0});
    auto objm = chunks.find({"ObjM", 1});
    if (objt == chunks.end() || objm == chunks.end()) return false;
    std::map<int, TypeEntry> types = ReadTypes(objt->second);
    for (const Placed& p : ReadObjects(objm->second)) {
        auto t = types.find(p.typeId);
        if (t == types.end()) continue;
        h.objects.push_back({p.id, t->second.guid, t->second.kind, ObjectType(t->second.guid), p.x, p.y, p.level,
                             p.container});
    }
    LogVerbose("house files: House%02d, lot %d tiles a side, %d types, %d objects", number, h.lotSize, (int)types.size(),
               (int)h.objects.size());
    *out = std::move(h);
    return true;
}

int FamilySize(int houseNumber) {
    if (houseNumber <= 0) return 0;
    std::string data;
    Chunks chunks;
    if (!ReadFile(NeighborhoodDir() + "Neighborhood.iff", &data) || !ReadChunks(data, &chunks)) return -1;
    // FAMI: u32 0, u32 version, "IMAF", i32 house number, i32 family number, i32 funds, i32
    // value, i32 x2, i32 member count, u32 GUID per member (the character's object type).
    for (const auto& [key, d] : chunks) {
        if (key.first != "FAMI" || d.size() < 40 || d.compare(8, 4, "IMAF") != 0 || (int)Le32(d, 12) != houseNumber)
            continue;
        int members = (int)Le32(d, 36), sims = 0;
        for (int i = 0; i < members && 40 + 4 * (size_t)i + 4 <= d.size(); i++) {
            // Pets' characters carry function flags 0x40 (all 20 of the first neighborhood; no
            // Sim does).
            Ptr type = ObjectType(Le32(d, 40 + 4 * i));
            sims += !(type && (TypeObjdField(type, addr::objd::kFunctionFlags) & kPetFunctionFlag));
        }
        return sims;
    }
    return 0;
}

}  // namespace game::housefile
