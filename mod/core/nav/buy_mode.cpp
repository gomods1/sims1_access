#include "buy_mode.h"

#include <algorithm>
#include <cstdlib>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/lotgrid.h"
#include "../game/rooms.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
#include "cues.h"
#include "camera.h"
#include "navigator.h"
#include "placement.h"
#include "room_names.h"
#include "world.h"

namespace nav::buy {
namespace {

struct State {
    game::Ptr session = 0;                // what the spots were found for: the product being bought,
                                          // or the object being moved (0: none yet)
    game::Ptr held = 0;                   // the held object they were shown with
    std::vector<placement::Area> areas;
    int area = 0;
    int spot = 0;
    // The arrows' free cursor: the held object goes wherever the player moves it, tile by tile
    // (for places the spot list leaves out); the engine says whether it fits there.
    bool free = false;
    game::Tile freeTile;
    int freeDirection = 0;
    bool freeFits = false;  // the engine let the held object stand on the free cursor's tile
    int freeRoom = -1;  // the room the free cursor's line named last (said again when it changes)
    // Ctrl+Shift+PageDown/PageUp: the lot's things nearest the cursor first (ids), and the one
    // the held object was put beside last.
    std::vector<int> things;
    int thing = -1;
};

State g;
// Where the object being moved stood: the arrows' cursor starts there.
game::Tile g_origin{-1, -1};
int g_originDirection = 0;

// The wall bit behind something facing an engine direction (6 north, 0 west, 2 south, 4 east).
uint32_t kWallBehind(int direction) {
    switch ((direction + 4) & 7) {
    case 6: return game::kWallNorth;
    case 0: return game::kWallWest;
    case 2: return game::kWallSouth;
    case 4: return game::kWallEast;
    }
    return 0;
}

const Str kRatings[game::kRatingCount] = {Str::RatingHunger, Str::RatingComfort, Str::RatingHygiene,
                                          Str::RatingBladder, Str::RatingEnergy, Str::RatingFun,
                                          Str::RatingRoom};
const Str kSkills[game::kSkillCount] = {Str::SkillCooking, Str::SkillMechanical, Str::SkillLogic, Str::SkillBody,
                                        Str::SkillCreativity, Str::SkillCharisma, Str::SkillStudy};

// "comodidad 2, energía 3, +lógica"; `all` lists every rating, else the first two.
std::string Ratings(const game::ProductInfo& p, bool all) {
    std::string s;
    int shown = 0;
    for (int i = 0; i < game::kRatingCount; i++) {
        if (p.ratings[i] == 0 || (!all && shown == 2)) continue;
        s += (s.empty() ? "" : ", ") + util::Format("%s %d", S(kRatings[i]), p.ratings[i]);
        shown++;
    }
    for (int i = 0; i < game::kSkillCount; i++)
        if (p.skillFlags & (1 << i)) s += (s.empty() ? "" : ", ") + std::string(S(kSkills[i]));
    return s;
}

// "$80, comodidad 2": what follows the product's name.
std::string ProductFacts(const game::ProductInfo& p, const std::string& price) {
    std::string s = price;
    int funds = 0;
    if (game::GetFunds(&funds) && p.price > funds) s += std::string(", ") + S(Str::CannotAfford);
    std::string ratings = Ratings(p, false);
    if (!ratings.empty()) s += ", " + ratings;
    return s;
}

std::string ProductLine(const game::ProductInfo& p, const std::string& price) {
    return p.name + ", " + ProductFacts(p, price);
}

placement::Area* CurrentArea() {
    if (g.areas.empty()) return nullptr;
    g.area = std::clamp(g.area, 0, (int)g.areas.size() - 1);
    return &g.areas[g.area];
}

const placement::Spot* CurrentSpot() {
    placement::Area* a = CurrentArea();
    if (!a || a->spots.empty()) return nullptr;
    g.spot = std::clamp(g.spot, 0, (int)a->spots.size() - 1);
    return &a->spots[g.spot];
}

// The held object shows on the focused spot (the tool puts it there on its next mouse move).
void ShowCurrent() {
    const placement::Spot* s = CurrentSpot();
    if (!s) {
        game::StopShowingHeld();
        return;
    }
    game::LiftHeld();
    if (s->direction >= 0) game::TurnHeld(s->direction);
    game::ShowHeldAt(s->tile);
    camera::LookAt(game::HeldObject());
    LogVerbose("spot %d,%d direction %d walls %x score %d, walls there %x", s->tile.x, s->tile.y, s->direction,
               s->wallSides, s->score, game::WallsAt(game::HeldLevel(), s->tile.x, s->tile.y));
}

std::string SpotLine() {
    placement::Area* a = CurrentArea();
    const placement::Spot* s = CurrentSpot();
    if (!a || !s) return "";
    return util::Format(S(Str::SpotOf), g.spot + 1, (int)a->spots.size(), placement::Describe(*s).c_str());
}

// How much floor the held object takes and along which way ("ocupa 6 casillas: 3 de oeste a
// este, 2 de norte a sur"): a tester's pet bed did not fit and nothing had said how big it was,
// and a bed "facing north" lay west to east.
std::string SizeLine(const std::vector<game::Tile>& footprint) {
    std::vector<game::Tile> tiles;  // parts can share a tile
    for (const game::Tile& t : footprint)
        if (std::find(tiles.begin(), tiles.end(), t) == tiles.end()) tiles.push_back(t);
    if (tiles.empty()) return "";
    int x0 = tiles[0].x, x1 = x0, y0 = tiles[0].y, y1 = y0;
    for (const game::Tile& t : tiles) {
        x0 = std::min(x0, t.x);
        x1 = std::max(x1, t.x);
        y0 = std::min(y0, t.y);
        y1 = std::max(y1, t.y);
    }
    return util::Format(S(Str::ObjectSize), (int)tiles.size(), x1 - x0 + 1, y1 - y0 + 1);
}

std::string SizeLine(const placement::Spot& s) { return SizeLine(s.footprint); }

// Names of the objects on these tiles (the held object's own parts left out), each once. Every
// object of a tile counts: a side table and the window in the wall beside it share one.
std::string ObjectsOn(const std::vector<game::Tile>& tiles, int level, game::Ptr held) {
    std::vector<game::Ptr> own = game::ObjectParts(held);
    std::vector<std::string> names;
    std::vector<game::PlacedPart> placed = game::PlacedParts();
    for (const game::Tile& t : tiles)
        for (const game::PlacedPart& p : placed) {
            if (p.level != level || p.tileX != t.x || p.tileY != t.y) continue;
            game::LotObject o;
            if (!game::GetObject(p.object, &o) || game::IsInvisibleHelper(o.object)) continue;
            if (o.object == held || std::find(own.begin(), own.end(), o.object) != own.end()) continue;
            if (game::IsKindOf(o.object, "cXCursorObject") || game::ObjectInternalName(o).rfind("Tile Cursor", 0) == 0) continue;
            std::string name = world::DisplayName(o);
            if (!name.empty() && std::find(names.begin(), names.end(), name) == names.end()) names.push_back(name);
        }
    std::string out;
    for (const std::string& n : names) out += (out.empty() ? "" : ", ") + n;
    return out;
}

// The tiles just in front of a footprint facing `direction` (what a chair would face), in the
// same room: a window in the wall behind is not what it faces.
std::vector<game::Tile> FrontTiles(const std::vector<game::Tile>& footprint, int direction, int level) {
    std::vector<game::Tile> out;
    for (const game::Tile& t : footprint) {
        game::Tile n = game::Step(t, direction);
        if (std::find(footprint.begin(), footprint.end(), n) != footprint.end()) continue;
        if (game::RoomAt(level, n.x, n.y) != game::RoomAt(level, t.x, t.y)) continue;
        out.push_back(n);
    }
    return out;
}

// Everything about the focused spot, for the details buffer: what is held, the spot's parts, its
// tile, and what else was checked in the area.
std::vector<std::string> SpotDetails() {
    std::vector<std::string> lines;
    placement::Area* a = CurrentArea();
    const placement::Spot* s = CurrentSpot();
    game::ProductInfo p;
    if (game::IsBuying() && game::HeldProduct(&p)) lines.push_back(ProductLine(p, util::Money(p.price)));
    if (a && s) {
        lines.push_back(util::Format(S(Str::SpotOf), g.spot + 1, (int)a->spots.size(), a->name.c_str()));
        for (const std::string& part : placement::DescribeParts(*s)) lines.push_back(part);
        std::string size = SizeLine(*s);
        if (!size.empty()) lines.push_back(size);
        lines.push_back(util::Format(S(Str::TileAt), s->tile.x, s->tile.y));
        lines.push_back(s->useTiles.size() == 1 ? std::string(S(Str::UsedFromTile))
                                                : util::Format(S(Str::UsedFromTiles), (int)s->useTiles.size()));
    } else if (a) {
        lines.push_back(util::Format(S(Str::AreaNoSpot), a->name.c_str(), placement::WhyNone(*a).c_str()));
    }
    if (a) {
        lines.push_back(util::Format(S(Str::AreaChecked), a->name.c_str(), (int)a->spots.size(), a->engineRefused,
                                     a->unusable, a->blocksDoor, a->blocksOthers));
        std::string why = a->commonError > 0 ? game::PlacementErrorText(a->commonError) : "";
        if (!why.empty()) lines.push_back(util::Format(S(Str::MostlyBecause), why.c_str()));
    }
    return lines;
}

// Speaks a spot or area line and puts its details in the buffer.
void SaySpot(const std::string& line) {
    buffers::SetDetails(SpotDetails());
    speech::Say(line);
}

std::string AreaLine() {
    placement::Area* a = CurrentArea();
    if (!a) return "";
    if (a->spots.empty()) return util::Format(S(Str::AreaNoSpot), a->name.c_str(), placement::WhyNone(*a).c_str());
    std::string s = a->spots.size() == 1 ? util::Format(S(Str::AreaSpotsOne), a->name.c_str())
                                         : util::Format(S(Str::AreaSpots), a->name.c_str(), (int)a->spots.size());
    return s + ". " + SpotLine();
}

game::Ptr Session() { return game::IsBuying() ? game::HeldProductType() : game::HeldObject(); }

std::string HeldName() {
    game::LotObject o;
    return game::GetObject(game::HeldObject(), &o) ? world::DisplayName(o) : "";
}

// Placing started (or the next one of a kind is in hand): find the spots and say the best.
void Start(game::Ptr held) {
    g = State{};
    g.held = held;
    g.session = Session();
    g.areas = placement::FindSpots();
    std::string line;
    game::ProductInfo p;
    if (game::IsBuying()) {
        if (game::HeldProduct(&p)) line = ProductLine(p, util::Money(p.price)) + ". ";
    } else {
        line = util::Format(S(Str::Moving), HeldName().c_str(), util::Money(game::SaleValue(held)).c_str()) + ". ";
    }
    bool any = false;
    for (const placement::Area& a : g.areas) any |= !a.spots.empty();
    if (!any) {
        std::string why = g.areas.empty() ? S(Str::DoesNotFit) : placement::WhyNone(g.areas.front());
        speech::Say(line + util::Format(S(Str::NoSpotAnywhere), why.c_str()));
        return;
    }
    ShowCurrent();
    const placement::Spot* first = CurrentSpot();
    if (first && first->footprint.size() > 1) line += SizeLine(*first) + ". ";
    SaySpot(line + AreaLine());
}

}  // namespace

void Reset() {
    if (g.session) game::StopShowingHeld();
    g = State{};
    if (!game::IsPlacing()) g_origin = game::Tile{-1, -1};
}

// The press reaches the tool a moment later: until then the old product is still the session,
// which would be searched (and spoken) once for nothing.
game::Ptr g_renewedFrom = 0;
ULONGLONG g_renewedAt = 0;
constexpr ULONGLONG kRenewWaitMs = 400;

ULONGLONG g_holdOffUntil = 0;

void HoldOff(unsigned ms) { g_holdOffUntil = GetTickCount64() + ms; }

void Renew() {
    g_holdOffUntil = 0;
    g_renewedFrom = g.session;
    g_renewedAt = GetTickCount64();
    g.session = 0;
}

bool Placing() { return game::IsPlacing() && game::HeldObject() != 0; }

// The game makes a new held object whenever the real mouse enters the lot; the same product
// keeps its spots and just shows again. A new product, or the next one after buying, gets new
// spots.
void Tick() {
    if (GetTickCount64() < g_holdOffUntil) {
        if (game::IsBuying()) {
            game::CancelPlacing();
            Reset();
        }
        return;
    }
    if (!game::IsPlacing()) {
        if (g.session) Reset();
        return;
    }
    if (!game::HeldObject() && !game::BeginPlacing()) {
        static ULONGLONG logged = 0;
        if (GetTickCount64() - logged > 5000) {
            logged = GetTickCount64();
            LogVerbose("buy: placing, but no held object");
        }
        return;
    }
    game::Ptr held = game::HeldObject();
    if (!held) return;
    if (Session() != g.session) {
        if (!g.session && Session() == g_renewedFrom && GetTickCount64() - g_renewedAt < kRenewWaitMs) return;
        Start(held);
    } else if (held != g.held) {
        g.held = held;
        ShowCurrent();
    }
}

void NextSpot(int delta) {
    g.free = false;
    placement::Area* a = CurrentArea();
    if (!a) {
        speech::Say(S(Str::DoesNotFit));
        return;
    }
    if (a->spots.empty()) {
        SaySpot(AreaLine());
        return;
    }
    g.spot = std::clamp(g.spot + delta, 0, (int)a->spots.size() - 1);  // the ends repeat
    ShowCurrent();
    SaySpot(SpotLine());
}

void NextArea(int delta) {
    g.free = false;
    if (g.areas.empty()) {
        speech::Say(S(Str::DoesNotFit));
        return;
    }
    int n = (int)g.areas.size();
    g.area = ((g.area + delta) % n + n) % n;
    g.spot = 0;
    ShowCurrent();
    SaySpot(AreaLine());
}

// The other directions that work on the focused tile.
void NextTurn(int delta) {
    if (FreeTurn(delta)) return;
    placement::Area* a = CurrentArea();
    const placement::Spot* s = CurrentSpot();
    if (!a || !s) {
        NextSpot(0);
        return;
    }
    std::vector<int> same;
    for (int i = 0; i < (int)a->spots.size(); i++)
        if (a->spots[i].tile == s->tile) same.push_back(i);
    std::sort(same.begin(), same.end(),
              [&](int x, int y) { return a->spots[x].direction < a->spots[y].direction; });
    if (same.size() < 2) {
        speech::Say(S(Str::NoOtherTurn));
        return;
    }
    int at = (int)(std::find(same.begin(), same.end(), g.spot) - same.begin());
    int n = (int)same.size();
    g.spot = same[((at + delta) % n + n) % n];
    ShowCurrent();
    SaySpot(SpotLine());
}

void Confirm() {
    const placement::Spot* s = CurrentSpot();
    if (!s && !g.free) {
        NextSpot(0);
        return;
    }
    game::Tile tile = g.free ? g.freeTile : s->tile;
    int direction = g.free ? g.freeDirection : s->direction;
    bool buying = game::IsBuying();
    std::string name = buying ? "" : HeldName();
    game::ProductInfo p;
    if (buying && game::HeldProduct(&p)) name = p.name;
    if (name.empty()) name = HeldName();  // build mode's items are not in the buy catalog
    game::Ptr placed = game::HeldObject();
    bool door = game::IsPortal(placed) && game::PeoplePass(placed) && direction >= 0 &&
                (game::WallsAt(game::HeldLevel(), tile.x, tile.y) & kWallBehind(direction)) != 0;
    game::LiftHeld();
    if (direction >= 0) game::TurnHeld(direction);
    int error = game::PlaceHeldAt(tile);
    if (error != 0) {
        std::string why = game::PlacementErrorText(error);
        buffers::Event(util::Format(S(Str::PlaceFailed), why.empty() ? S(Str::DoesNotFit) : why.c_str()));
        if (g.free) game::ShowHeldAt(tile);
        else ShowCurrent();
        return;
    }
    g.free = false;
    if (buying) world::MarkBought(placed);
    std::string line = util::Format(S(buying ? Str::Bought : Str::PutDown), name.c_str());
    int funds = 0;
    if (buying && game::GetFunds(&funds)) line += ". " + util::Format(S(Str::FundsLeft), util::Money(funds).c_str());
    if (door) {  // the placing is over, and only now can the engine's own state answer this
        std::string blocker;
        int check = placement::CheckDoor(tile, direction, game::HeldLevel(), &blocker);
        if (check == 1) line += ". " + util::Format(S(Str::DoorBlocked), blocker.c_str());
        else if (check == 2) line += ". " + std::string(S(Str::DoorNoWay));
        else line += ". " + std::string(S(Str::DoorUsable));
    } else if (!game::IsPortal(placed)) {
        // How it will be used, as the door check says for doors (a tester could not tell whether
        // his Sim would reach the computer on the desk or sit at it).
        game::LotObject po;
        bool seat = game::GetObject(placed, &po) && (po.functionFlags & game::kFnSeating);
        std::string with = seat ? placement::SeatServesNow(placed) : placement::ServedByNow(placed);
        int use = placement::CanUseNow(placed);
        if (!with.empty()) line += ". " + util::Format(S(seat ? Str::SeatFor : Str::UsedFromSeat), with.c_str());
        else if (use == 2) line += ". " + std::string(S(Str::NobodyReaches));
        else if (use == 1) line += ". " + std::string(S(Str::SimsCanUse));
    }
    g.session = 0;  // the lot changed: the next one of a kind gets new spots
    if (Placing()) line += ". " + std::string(S(Str::AnotherInHand));
    buffers::Event(line);
}

bool Cancel() {
    if (!game::IsPlacing()) return false;
    if (game::IsBuying()) {
        game::CancelPlacing();
        Reset();
        buffers::Event(S(Str::PlacingCancelled));
        return true;
    }
    std::string name = HeldName();
    game::PutBackHeld();
    Reset();
    buffers::Event(util::Format(S(Str::PutBack), name.c_str()));
    return true;
}

void Sell() {
    if (game::IsBuying()) {
        Cancel();
        return;
    }
    std::string name = HeldName();
    int before = 0, after = 0;
    bool counted = game::GetFunds(&before);
    if (!game::SellHeld() || game::HeldObject()) {
        speech::Say(S(Str::CannotSell));
        return;
    }
    Reset();
    if (counted && game::GetFunds(&after) && after > before)
        buffers::Event(util::Format(S(Str::Sold), name.c_str(), util::Money(after - before).c_str(),
                                    util::Money(after).c_str()));
    else
        buffers::Event(util::Format(S(Str::ThrownAway), name.c_str()));
}

void PickUpFocused() {
    game::Ptr object = world::FocusedObject();
    if (!object) {
        speech::Say(S(Str::NoObjects));
        return;
    }
    game::LotObject o;
    if (game::GetObject(object, &o)) {
        placement::SetMoveOrigin(game::Tile{o.x >> 4, o.y >> 4}, o.level);
        g_origin = game::Tile{o.x >> 4, o.y >> 4};
        g_originDirection = game::ObjectDirection(object);
    }
    int error = game::PickUp(object);
    if (error != 0) {
        std::string why = game::PlacementErrorText(error);
        std::string line = util::Format(S(Str::CannotPickUp), why.c_str());
        if (game::IsPortal(object)) line += ". " + std::string(S(Str::PortalDeleteHint));
        speech::Say(line);
    }
    // Tick notices the held object and says where it can go.
}

bool RemoveFocusedPortal() {
    game::Ptr object = world::FocusedObject();
    if (!object || !game::IsPortal(object)) return false;
    if (!game::InBuyTool()) {
        speech::Say(S(Str::PortalNeedsHand));
        return true;
    }
    game::LotObject o;
    std::string name = game::GetObject(object, &o) ? world::DisplayName(o) : "";
    world::Refocus();
    if (!game::RemovePortal(object)) {
        speech::Say(util::Format(S(Str::PortalNotRemoved), name.c_str()));
        return true;
    }
    buffers::Event(util::Format(S(Str::PortalRemoved), name.c_str()));
    return true;
}

// ---- The free cursor (arrows while placing) ------------------------------------------------

namespace {

// "cabe, baño, paredes al norte, mirando al este, casilla 29, 31": the engine's verdict first,
// the room when the cursor has just entered it, the walls, the facing; the tile last (a tester,
// 2026-10-31: coordinates first made the line hard to follow).
std::string FreeLine(bool withRoom = false) {
    int level = game::HeldLevel();
    game::Ptr held = game::HeldObject();
    game::LiftHeld();
    game::TurnHeld(g.freeDirection);
    int error = game::CheckHeldAt(g.freeTile, level);
    g.freeFits = error == 0;
    game::ShowHeldAt(g.freeTile);
    camera::LookAt(held);
    std::vector<game::Tile> footprint = game::FootprintAt(held, g.freeTile);
    std::string line;
    if (error == 0) line = S(Str::FreeFits);
    else {
        std::string why = game::PlacementErrorText(error);
        line = util::Format(S(Str::FreeNoFit), why.empty() ? S(Str::DoesNotFit) : why.c_str());
        // what it runs into (a tester: it may not overlap another object, but which one?)
        std::string in = ObjectsOn(footprint, level, held);
        if (!in.empty()) line += ", " + util::Format(S(Str::CollidesWith), in.c_str());
    }
    int roomId = game::RoomAt(level, g.freeTile.x, g.freeTile.y);
    if (withRoom || roomId != g.freeRoom) {
        std::string from = g.freeRoom >= 0 && !withRoom ? placement::RoomNameOf(g.freeRoom) : "";
        std::string room = placement::RoomNameOf(roomId);
        g.freeRoom = roomId;
        if (!room.empty() && !from.empty() && from != room)
            line += ", " + util::Format(S(Str::RoomChange), from.c_str(), room.c_str());
        else if (!room.empty())
            line += ", " + room;
    }
    uint32_t walls = game::WallsAt(level, g.freeTile.x, g.freeTile.y);
    std::string sides;
    const std::pair<uint32_t, Str> kSides[] = {{game::kWallNorth, Str::BuildNorth}, {game::kWallEast, Str::BuildEast},
                                              {game::kWallSouth, Str::BuildSouth}, {game::kWallWest, Str::BuildWest}};
    for (const auto& side : kSides)
        if (walls & side.first) sides += (sides.empty() ? "" : ", ") + std::string(S(side.second));
    if (!sides.empty()) line += ", " + util::Format(S(Str::BuildWallsHere), sides.c_str());
    else {  // the whole footprint away from any wall: what a tester's pet bed was, in mid-room
        bool touches = false;
        for (const game::Tile& t : footprint) touches |= game::WallsAt(level, t.x, t.y) != 0;
        if (!touches && game::RoomAt(level, g.freeTile.x, g.freeTile.y) != game::kNoRoom) {
            game::Room info;
            if (game::GetRoom(game::RoomAt(level, g.freeTile.x, g.freeTile.y), &info) && !info.outside)
                line += std::string(", ") + S(Str::AwayFromWalls);
        }
    }
    line += ", " + util::Format(S(Str::FacingDirection), S(placement::CompassWord(game::CompassOf(g.freeDirection))));
    // A door or window goes in the wall behind it: this is what turning it chooses.
    if (error == 0 && game::IsPortal(held) && (walls & kWallBehind(g.freeDirection))) {
        game::Tile behind = game::Step(g.freeTile, (g.freeDirection + 4) & 7);
        line += ", " + util::Format(S(Str::DoorInWall), S(placement::CompassWord(game::CompassOf((g.freeDirection + 4) & 7))),
                                    placement::RoomNameOf(roomId).c_str(),
                                    placement::RoomNameOf(game::RoomAt(level, behind.x, behind.y)).c_str());
    }
    std::string faced = ObjectsOn(FrontTiles(footprint, g.freeDirection, level), level, held);
    if (!faced.empty()) line += ", " + util::Format(S(Str::FacingThing), faced.c_str());
    return line + ", " + util::Format(S(Str::TileAt), g.freeTile.x, g.freeTile.y);
}

}  // namespace

void MoveFree(int dx, int dy, bool throughWalls) {
    if (!Placing()) return;
    if (!g.free) {
        const placement::Spot* s = CurrentSpot();
        game::LotObject sim;
        if (!game::IsBuying() && g_origin.x >= 0) {  // a moved object: from where it stood
            g.freeTile = g_origin;
            g.freeDirection = g_originDirection;
            g.free = true;
            g.freeRoom = -1;
            std::string line = FreeLine(true);
            buffers::SetDetails({line});
            speech::Say(util::Format(S(Str::FromWhereItWas), line.c_str()));
            cues::Tile(game::HeldLevel(), g.freeTile.x, g.freeTile.y, game::HeldObject());
            cues::Fit(g.freeFits);
            g_origin = game::Tile{-1, -1};  // later arrows go from the cursor
            return;
        }
        if (s) {
            g.freeTile = s->tile;
            g.freeDirection = s->direction >= 0 ? s->direction : game::ObjectDirection(game::HeldObject());
        } else if (game::GetObject(game::SelectedSim(), &sim)) {
            g.freeTile = game::Tile{sim.x >> 4, sim.y >> 4};
            g.freeDirection = game::ObjectDirection(game::HeldObject());
        }
        g.free = true;
    }
    // One tile at a time: a wall, fence or window stops the cursor (a door does not), unless Alt
    // is held, so a room is not left by accident (a tester).
    int steps = std::max(std::abs(dx), std::abs(dy));
    int sx = dx > 0 ? 1 : dx < 0 ? -1 : 0, sy = dy > 0 ? 1 : dy < 0 ? -1 : 0;
    int moved = 0;
    std::string wall;
    for (int i = 0; i < steps; i++) {
        game::Tile next{g.freeTile.x + sx, g.freeTile.y + sy};
        if (!game::InGrid(next.x, next.y)) break;
        if (!throughWalls && cues::WallStops(game::HeldLevel(), g.freeTile.x, g.freeTile.y, sx, sy, &wall)) break;
        g.freeTile = next;
        moved++;
    }
    if (moved == 0) {
        if (!wall.empty()) {
            speech::Say(wall);
            cues::Blocked();
        } else {
            speech::Say(S(Str::BuildLotEdge));
            cues::LotEdge();
        }
        return;
    }
    std::string line = FreeLine();
    if (!wall.empty()) line += ". " + wall;
    buffers::SetDetails({line});
    speech::Say(line);
    // The tile's sounds, then whether the object fits on it (nav/cues.h).
    cues::Tile(game::HeldLevel(), g.freeTile.x, g.freeTile.y, game::HeldObject());
    cues::Fit(g.freeFits);
}

void NextThing(int delta) {
    if (!Placing()) return;
    game::Ptr held = game::HeldObject();
    int level = game::HeldLevel();
    std::vector<game::Ptr> own = game::ObjectParts(held);
    if (g.things.empty()) {  // nearest the cursor (or the spot, or the Sim) first
        game::Tile origin{0, 0};
        const placement::Spot* spot = CurrentSpot();
        game::LotObject sim;
        if (g.free) origin = g.freeTile;
        else if (spot) origin = spot->tile;
        else if (game::GetObject(game::SelectedSim(), &sim)) origin = game::Tile{sim.x >> 4, sim.y >> 4};
        std::vector<std::pair<int, int>> byDistance;  // (squared distance, id)
        for (const game::LotObject& o : game::LotObjects()) {
            if (o.person || o.level != level || o.object == held || std::find(own.begin(), own.end(), o.object) != own.end())
                continue;
            int dx = (o.x >> 4) - origin.x, dy = (o.y >> 4) - origin.y;
            byDistance.push_back({dx * dx + dy * dy, o.id});
        }
        std::sort(byDistance.begin(), byDistance.end());
        for (const auto& d : byDistance) g.things.push_back(d.second);
        g.thing = -1;
    }
    if (g.things.empty()) {
        speech::Say(S(Str::NoObjects));
        return;
    }
    int n = (int)g.things.size();
    g.thing = g.thing < 0 ? (delta > 0 ? 0 : n - 1) : ((g.thing + delta) % n + n) % n;
    game::LotObject target;
    if (!game::GetObject(game::ObjectById(g.things[g.thing]), &target)) {
        g.things.clear();
        speech::Say(S(Str::NoObjects));
        return;
    }
    // A free tile beside it: facing it if the held object fits that way, else any way it fits;
    // when nothing fits, the first tile beside it, so the line says why.
    // where its parts stand (FootprintAt is for the held object: a placed one may be turned)
    std::vector<game::Ptr> parts = game::ObjectParts(target.object);
    std::vector<game::Tile> body;
    for (const game::PlacedPart& p : game::PlacedParts())
        if (p.level == level && (p.object == target.object || std::find(parts.begin(), parts.end(), p.object) != parts.end()))
            body.push_back(game::Tile{p.tileX, p.tileY});
    if (body.empty()) body.push_back(game::Tile{target.x >> 4, target.y >> 4});
    game::LiftHeld();
    bool found = false;
    game::Tile firstTile{-1, -1};
    int firstFacing = g.free ? g.freeDirection : game::ObjectDirection(held);
    for (int pass = 0; pass < 2 && !found; pass++)
        for (const game::Tile& f : body) {
            for (int dir = 0; dir < 8 && !found; dir += 2) {
                game::Tile t = game::Step(f, dir);
                if (!game::InGrid(t.x, t.y) || std::find(body.begin(), body.end(), t) != body.end()) continue;
                if (game::RoomAt(level, t.x, t.y) != game::RoomAt(level, f.x, f.y)) continue;  // not through a wall
                int facing = (dir + 4) % 8;  // from the tile toward the thing
                if (firstTile.x < 0) {
                    firstTile = t;
                    firstFacing = facing;
                }
                for (int k = 0; k < (pass == 0 ? 1 : 4) && !found; k++) {
                    int d = (facing + 2 * k) % 8;
                    if (!game::TurnHeld(d) || game::CheckHeldAt(t, level) != 0) continue;
                    found = true;
                    g.freeTile = t;
                    g.freeDirection = d;
                }
            }
            if (found) break;
        }
    if (!found && firstTile.x >= 0) {
        g.freeTile = firstTile;
        g.freeDirection = firstFacing;
    }
    g.free = true;
    std::string line = util::Format(S(Str::BesideThing), world::DisplayName(target).c_str()) + ": " + FreeLine(true);
    buffers::SetDetails({line});
    speech::Say(line);
}

bool FreeTurn(int delta) {
    if (!g.free) return false;
    int step = game::HeldRotationStep();
    if (step <= 0) {
        speech::Say(S(Str::NoOtherTurn));
        return true;
    }
    g.freeDirection = ((g.freeDirection + delta * std::max(step, 2)) % 8 + 8) % 8;
    std::string line = FreeLine();
    buffers::SetDetails({line});
    speech::Say(line);
    return true;
}

void Repeat() {
    if (g.free) {
        std::string line = FreeLine(true);
        buffers::SetDetails({line});
        speech::Say(line);
        return;
    }
    if (!CurrentSpot()) {
        SaySpot(AreaLine());
        return;
    }
    SaySpot(SpotLine());
}

// What the catalog's popup shows on hover: name, price, ratings, then its description. The
// button's role and state follow the name: "Silla Nalghaus, botón, $80, comodidad 2. ...".
std::string ProductText(game::Ptr button) {
    std::string role = std::string(", ") + S(Str::RoleButton);
    game::ProductInfo p;
    if (!game::ReadProductOfButton(button, &p)) return game::Tooltip(button) + role;
    std::string price = game::Tooltip(button);  // the game's own money format
    std::string s = p.name + role;
    if (game::ButtonChecked(button)) s += std::string(", ") + S(Str::Selected);
    s += ", " + ProductFacts(p, price.empty() ? util::Money(p.price) : price);
    if (!p.description.empty()) s += ". " + p.description;
    return s;
}

// The rooms buy mode suggests it for (OBJD room flags): "para cocina, comedor".
std::string SuggestedRooms(uint16_t flags) {
    const Str kinds[] = {Str::RoomKitchen, Str::RoomBedroom, Str::RoomBathroom, Str::RoomLiving,
                         Str::RoomOutside, Str::RoomDining, Str::RoomStudy};
    std::string s;
    for (Str kind : kinds)
        if (rooms::Suits(kind, flags)) s += (s.empty() ? "" : ", ") + std::string(S(kind));
    return s.empty() ? s : util::Format(S(Str::MeantFor), s.c_str());
}

std::vector<std::string> ProductDetails(game::Ptr button) {
    game::ProductInfo p;
    if (!game::ReadProductOfButton(button, &p)) return {game::Tooltip(button)};
    std::string price = game::Tooltip(button);
    std::string line = p.name + ", " + (price.empty() ? util::Money(p.price) : price);
    int funds = 0;
    if (game::GetFunds(&funds) && p.price > funds) line += std::string(", ") + S(Str::CannotAfford);
    std::vector<std::string> lines = {line};
    for (int i = 0; i < game::kRatingCount; i++)
        if (p.ratings[i] != 0) lines.push_back(util::Format("%s %d", S(kRatings[i]), p.ratings[i]));
    for (int i = 0; i < game::kSkillCount; i++)
        if (p.skillFlags & (1 << i)) lines.push_back(S(kSkills[i]));
    std::string rooms = SuggestedRooms(p.roomFlags);
    if (!rooms.empty()) lines.push_back(rooms);
    for (const std::string& sentence : buffers::Sentences(p.description)) lines.push_back(sentence);
    return lines;
}

}  // namespace nav::buy
