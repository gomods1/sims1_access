#include "buy_mode.h"

#include <algorithm>

#include "../game/buy.h"
#include "../game/lot.h"
#include "../game/ui.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buffers.h"
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
};

State g;

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
    FocusWorld(true);
    SaySpot(line + AreaLine());
}

}  // namespace

void Reset() {
    if (g.session) game::StopShowingHeld();
    g = State{};
}

void Renew() { g.session = 0; }

bool Placing() { return game::IsPlacing() && game::HeldObject() != 0; }

// The game makes a new held object whenever the real mouse enters the lot; the same product
// keeps its spots and just shows again. A new product, or the next one after buying, gets new
// spots.
void Tick() {
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
        Start(held);
    } else if (held != g.held) {
        g.held = held;
        ShowCurrent();
    }
}

void NextSpot(int delta) {
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
    if (!s) {
        NextSpot(0);
        return;
    }
    game::Tile tile = s->tile;
    bool buying = game::IsBuying();
    std::string name = buying ? "" : HeldName();
    game::ProductInfo p;
    if (buying && game::HeldProduct(&p)) name = p.name;
    if (name.empty()) name = HeldName();  // build mode's items are not in the buy catalog
    game::Ptr placed = game::HeldObject();
    game::LiftHeld();
    if (s->direction >= 0) game::TurnHeld(s->direction);
    int error = game::PlaceHeldAt(tile);
    if (error != 0) {
        std::string why = game::PlacementErrorText(error);
        buffers::Event(util::Format(S(Str::PlaceFailed), why.empty() ? S(Str::DoesNotFit) : why.c_str()));
        ShowCurrent();
        return;
    }
    if (buying) world::MarkBought(placed);
    std::string line = util::Format(S(buying ? Str::Bought : Str::PutDown), name.c_str());
    int funds = 0;
    if (buying && game::GetFunds(&funds)) line += ". " + util::Format(S(Str::FundsLeft), util::Money(funds).c_str());
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
    int error = game::PickUp(object);
    if (error != 0) {
        std::string why = game::PlacementErrorText(error);
        speech::Say(util::Format(S(Str::CannotPickUp), why.c_str()));
    }
    // Tick notices the held object and says where it can go.
}

void Repeat() {
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
