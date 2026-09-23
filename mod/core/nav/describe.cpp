#include "describe.h"

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../game/addresses.h"
#include "../game/design.h"
#include "../game/house.h"
#include "../game/lot.h"
#include "../game/neighborhood.h"
#include "../game/people.h"
#include "../game/rtti.h"
#include "../game/ui.h"
#include "../game/view_control.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buy_mode.h"
#include "screens.h"
#include "tutorial.h"

namespace nav {
namespace {

std::unordered_map<int, std::string> g_houseCards;  // by house number

std::string HouseText(const Item& it) {
    auto cached = g_houseCards.find(it.number);
    if (cached != g_houseCards.end()) return cached->second;
    std::string text = game::LotCardText(it.widget);
    if (text.empty()) {
        text = util::Format(S(Str::HouseNumber), it.number);
        std::string family = game::LotFamily(it.widget);
        if (!family.empty()) text += ", " + family;
    }
    g_houseCards[it.number] = text;
    return text;
}

// "Calle Sim,1, botón. La familia Compañeras. Valor neto: $20.000": the role follows the card's
// first line, the house's address.
// On a travel map a community lot adds what the filter bar marks it for: "... Haz clic aquí
// para visitarla. En los filtros: Mostrar Tiendas de Jardinería".
std::string LotText(const Item& it) {
    std::string text = HouseText(it);
    std::string role = std::string(", ") + S(Str::RoleButton);
    size_t lineEnd = text.find_first_of("\r\n");
    text = lineEnd == std::string::npos ? text + role : text.insert(lineEnd, role);
    Ptr map = game::TravelMap(it.widget);
    if (!map || !game::IsCommunityLot(it.number)) return text;
    std::string filters;
    int bits = game::LotFilterBits(it.widget);
    for (const game::MapFilter& f : game::MapFilters(map))
        if (bits & f.bit) filters += (filters.empty() ? "" : ", ") + game::Tooltip(f.button);
    if (filters.empty()) return text;
    while (!text.empty() && (text.back() == ' ' || text.back() == '\n' || text.back() == '\r')) text.pop_back();
    text += text.empty() || text.back() == '.' ? " " : ". ";
    text += util::Format(S(Str::LotFilters), filters.c_str());
    return text;
}

// A lot's own name from its card: the line after the address, up to its first sentence's end
// ("El Paraíso de las Mascotas"); the address when the card has no name.
std::string LotPlaceName(const Item& it) {
    std::string text = HouseText(it);
    size_t lineEnd = text.find_first_of("\r\n");
    if (lineEnd == std::string::npos) return text;
    std::string address = text.substr(0, lineEnd);
    size_t start = text.find_first_not_of("\r\n", lineEnd);
    if (start == std::string::npos) return address;
    size_t end = text.find_first_of(".\r\n", start);
    std::string name = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
    return name.empty() ? address : name;
}

// "Gentil, botón, seleccionado. 4 miembros: Federico, Diana, Daniel, Virginia. valor neto:
// $20.000. amigos: 0", from the row's portraits and its three texts (name, money, friends; left
// to right).
std::string FamilyRowText(const Item& it) {
    std::vector<int> ids = game::FamilyRowMemberIds(it.widget);
    std::string s = game::FamilyRowName(it.widget) + ", " + S(Str::RoleButton);
    if (game::ButtonChecked(it.widget)) s += std::string(", ") + S(Str::Selected);
    s += ". ";
    s += ids.size() == 1 ? S(Str::FamilyMemberOne) : util::Format(S(Str::FamilyMembers), (int)ids.size());
    const char* separator = ": ";
    for (int id : ids) {
        std::string name = game::NeighborName(id);
        if (name.empty()) continue;
        s += separator + name;
        separator = ", ";
    }
    std::vector<std::pair<long, std::string>> texts;
    for (Ptr c : game::Children(it.widget))
        if (game::IsKindOf(c, "cTSWinText")) texts.push_back({game::Rect(c).left, game::Caption(c)});
    std::sort(texts.begin(), texts.end());
    if (texts.size() >= 3) {
        s += std::string(". ") + S(Str::NetWorth) + ": " + texts[1].second;
        s += std::string(". ") + S(Str::Friends) + ": " + texts[2].second;
    }
    return s;
}

// "Federico. Pulcritud 5, Extroversión 3, Actividad 7, Ánimo lúdico 4, Cordialidad 6. <bio>".
std::string PickerMemberText(const Item& it) {
    std::string s = game::NeighborName(it.number);
    int traits[game::kTraitCount];
    if (game::Personality(it.number, traits)) {
        const Str kNames[game::kTraitCount] = {Str::TraitNeat, Str::TraitOutgoing, Str::TraitActive,
                                               Str::TraitPlayful, Str::TraitNice};
        const char* separator = ". ";
        for (int i = 0; i < game::kTraitCount; i++) {
            s += separator + util::Format("%s %d", S(kNames[i]), traits[i]);
            separator = ", ";
        }
    }
    std::string bio = util::CleanForSpeech(game::NeighborBio(it.number));
    if (!bio.empty()) s += ". " + bio;
    return s;
}

// Column names come from the dialog's header row: the row whose columns are not money.
void BudgetColumnNames(Ptr row, std::string* first, std::string* second) {
    for (Ptr sibling : game::Children(game::Parent(row))) {
        if (!game::IsKindOf(sibling, "cWinBudgetRow")) continue;
        game::BudgetRow r = game::ReadBudgetRow(sibling);
        if (!r.column1.empty() && r.column1[0] != '$' && r.column1[0] != '-') {
            *first = r.column1;
            *second = r.column2;
            return;
        }
    }
}

// "Empleo. 3 días: $0. Hoy: $0"; a summary row reads its sentence; section titles read alone.
std::string BudgetRowText(const Item& it) {
    game::BudgetRow r = game::ReadBudgetRow(it.widget);
    if (!r.summary.empty()) return r.summary;
    std::string first, second;
    BudgetColumnNames(it.widget, &first, &second);
    if (!first.empty() && r.column1 == first && r.column2 == second) return first + ", " + second;  // header row
    std::string s = it.label;
    auto append = [&s](const std::string& part) {
        if (part.empty()) return;
        s += (s.empty() || s.back() == ':') ? " " : ". ";
        s += part;
    };
    append(first.empty() || r.column1.empty() ? r.column1 : first + ": " + r.column1);
    append(second.empty() || r.column2.empty() ? r.column2 : second + ": " + r.column2);
    return s;
}

std::string NameOf(const Item& it) { return it.label.empty() ? S(Str::Unnamed) : it.label; }

// Name, role, then value and state, the order a screen reader uses: "Guardar, botón";
// "Opciones de sonido, pestaña, seleccionada"; "Signo del zodiaco, botón, (Cáncer)" (the caption,
// when the label is the tooltip, can hold a value). A push button that stays checked is the chosen
// one of a set (skin tone, sex on create a Sim). Widgets that are not buttons only show hover
// text and say no role.
std::string ButtonText(const Item& it) {
    bool button = game::IsKindOf(it.widget, "cTSWinBtn");
    game::ButtonType type = button ? game::GetButtonType(it.widget) : game::ButtonType::Push;
    std::string s = it.label.empty() ? S(Str::UnlabeledButton) : it.label;
    if (button && !it.label.empty()) {
        Str role = it.role == Role::Tab ? Str::RoleTab
                   : type == game::ButtonType::Check ? Str::RoleCheckBox
                                                      : Str::RoleButton;
        s += std::string(", ") + S(role);
    }
    std::string caption = util::CleanForSpeech(game::Caption(it.widget));
    if (!caption.empty() && caption != it.label) s += ", " + caption;
    if (!button) return s;
    bool checked = game::ButtonChecked(it.widget);
    if (it.role == Role::Tab) {
        if (checked) s += std::string(", ") + S(Str::TabSelected);
    } else if (type == game::ButtonType::Check) {
        s += std::string(", ") + S(checked ? Str::Checked : Str::NotChecked);
    } else if (checked) {
        s += std::string(", ") + S(Str::Selected);
    }
    if (!game::ButtonEnabled(it.widget)) s += std::string(", ") + S(Str::Unavailable);
    return s;
}

// "Grado de humor, 3 de 5 barras en verde": the bars a sighted player sees on the meter.
std::string MoodText(const Item& it) {
    int mood = 0;
    if (!game::SimMood(game::SimNeighborId(game::SelectedSim()), &mood)) return it.label;
    int bars = game::MoodBars(mood);
    if (bars > 0) return util::Format(S(Str::MoodGood), it.label.c_str(), bars);
    if (bars < 0) return util::Format(S(Str::MoodBad), it.label.c_str(), -bars);
    return util::Format(S(Str::MoodNeutral), it.label.c_str());
}

// "Marisa Compañeras. Diaria 23, de por vida 20".
std::string RelationshipText(const Item& it) {
    game::Relationship r = game::ReadRelationship(it.widget);
    return util::Format(S(Str::RelationshipValues), it.label.c_str(), r.daily, r.lifetime);
}

// "Cocina, 3 de 10": the bars the job panel draws for the selected Sim.
std::string SkillText(const Item& it) {
    int points = 0;
    if (!game::SkillPoints(game::SimNeighborId(game::SelectedSim()), it.number, &points)) return it.label;
    return util::Format(S(Str::SkillValue), it.label.c_str(), points / 100);
}

// "Decano de universidad"; "sueldo: $700"; "rendimiento: Excelente"; "amigos de la familia:
// Necesarios 9". The job line is its own name; the panel shows icons for the others.
std::string JobInfoText(const Item& it) {
    std::string value = game::Caption(it.widget);
    Str name;
    switch ((JobField)it.number) {
    case JobField::Job: return value.empty() ? S(Str::UnlabeledButton) : value;
    case JobField::Pay: name = Str::JobPay; break;
    case JobField::Performance: name = Str::JobPerformance; break;
    default: name = Str::JobFriendsNeeded; break;
    }
    return value.empty() ? S(name) : util::Format(S(Str::LabelValue), S(name), value.c_str());
}

// "Pulcritud, 7 de 10".
std::string TraitText(const Item& it) {
    int points = 0;
    if (!game::TraitPoints(it.widget, &points)) return it.label;
    return util::Format(S(Str::SkillValue), it.label.c_str(), points);
}

// "11:46"; "Fondos: $9.500, botón" (it opens the budget); "Pausa"; "Recuento de amigos de la
// familia: 0". Read live: the clock and funds change while the item is focused.
std::string StatusText(const Item& it) {
    switch ((StatusField)it.number) {
    case StatusField::Clock:
        return game::ClockText(it.widget);
    case StatusField::Speed:
        return GameSpeedName(game::Parent(it.widget));
    case StatusField::Funds:
        return util::Format(S(Str::LabelValue), it.label.c_str(), game::Caption(it.widget).c_str()) + ", " +
               S(Str::RoleButton);
    case StatusField::Friends:
        return util::Format(S(Str::LabelValue), it.label.c_str(), game::Caption(it.widget).c_str());
    }
    return it.label;
}

// The word after the codes: "c036malgt_dave" -> "dave", "c617malgt_rocketsoldier_enlisted" ->
// "rocketsoldier enlisted".
std::string LookWord(const std::string& mesh) {
    size_t underscore = mesh.find('_');
    std::string word = underscore == std::string::npos ? mesh : mesh.substr(underscore + 1);
    for (char& c : word)
        if (c == '_') c = ' ';
    return util::CleanForSpeech(word);
}

// Bodies carry a shape after sex and age: "b011mafitlgt_..." -> fit. Children's bodies have none.
bool BodyShape(const std::string& mesh, Str* shape) {
    std::string code = mesh.substr(0, mesh.find('_'));
    if (code.find("fat") != std::string::npos) *shape = Str::BodyFat;
    else if (code.find("skn") != std::string::npos) *shape = Str::BodySkinny;
    else if (code.find("fit") != std::string::npos) *shape = Str::BodyFit;
    else return false;
    return true;
}

}  // namespace

std::string DescribeLook(Ptr screen, game::LookPart part) {
    game::Look look;
    if (!game::CurrentLook(screen, part, &look)) return "";
    std::string word = LookWord(look.mesh);
    if (part == game::LookPart::Head)
        return util::Format(S(Str::HeadLook), look.index + 1, look.count, word.c_str());
    Str shape;
    if (!BodyShape(look.mesh, &shape))
        return util::Format(S(Str::BodyLookNoShape), look.index + 1, look.count, word.c_str());
    return util::Format(S(Str::BodyLook), look.index + 1, look.count, S(shape), word.c_str());
}

// Pet bodies name their breed after the species: "b002dog_dalmation".
struct Breed {
    const char* mesh;
    Str name;
};
const Breed kBreeds[] = {
    {"dog_lab", Str::BreedLabrador}, {"dog_aussie", Str::BreedAussie}, {"dog_dalmation", Str::BreedDalmatian},
    {"dog_cairn", Str::BreedCairn}, {"dog_poodle", Str::BreedPoodle}, {"dog_jack", Str::BreedJackRussell},
    {"dog_springer", Str::BreedSpringer}, {"dog_setter", Str::BreedSetter}, {"dog_greyhound", Str::BreedGreyhound},
    {"dog_beagle", Str::BreedBeagle}, {"dog_rott", Str::BreedRottweiler}, {"dog_bulldog", Str::BreedBulldog},
    {"dog_scottish", Str::BreedScottie}, {"dog_shep", Str::BreedShepherd}, {"dog_chihuahua", Str::BreedChihuahua},
    {"dog_pinkpoodle", Str::BreedPinkPoodle},
    {"cat_tabby", Str::BreedTabby}, {"cat_silver", Str::BreedSilverTabby}, {"cat_calico", Str::BreedCalico},
    {"cat_persian", Str::BreedPersian}, {"cat_bluemanx", Str::BreedBlueManx}, {"cat_ragdoll", Str::BreedRagdoll},
    {"cat_siamese", Str::BreedSiamese}, {"cat_tortoise", Str::BreedTortoiseshell}, {"cat_bombay", Str::BreedBombay},
    {"cat_spotted", Str::BreedSpotted}, {"cat_calicoblk", Str::BreedBlackCalico}, {"cat_calicored", Str::BreedRedCalico},
    {"cat_sphinx", Str::BreedSphynx}, {"cat_albino", Str::BreedWhite}, {"cat_tabbybrown", Str::BreedBrownTabby},
};

std::string BreedName(const std::string& mesh) {
    size_t underscore = mesh.find('_');
    size_t start = underscore == std::string::npos ? 0 : mesh.find_last_of("0123456789", underscore) + 1;
    std::string key = mesh.substr(start);
    for (const Breed& b : kBreeds)
        if (key == b.mesh) return S(b.name);
    return LookWord(mesh);
}

// The wardrobe: "traje 3 de 12: fit, beachguy" (the body words of create-a-Sim); the pet picker:
// "mascota 2 de 16: dálmata".
std::string DescribeOutfit(Ptr dialog) {
    game::Look look;
    if (!game::CurrentOutfit(dialog, &look)) return "";
    if (game::IsPetDialog(dialog)) {
        if (look.index < 0) return "";
        return util::Format(S(Str::PetLook), look.index + 1, look.count, BreedName(look.mesh).c_str());
    }
    if (look.index < 0) return util::Format(S(Str::OutfitNotListed), look.count);
    std::string word = LookWord(look.mesh);
    Str shape;
    if (!BodyShape(look.mesh, &shape)) return util::Format(S(Str::OutfitLookNoShape), look.index + 1, look.count, word.c_str());
    return util::Format(S(Str::OutfitLook), look.index + 1, look.count, S(shape), word.c_str());
}

// A travel map's filter puts markers on the lots of its kind: they are said by name.
std::string DescribeMarkedLots(Ptr map) {
    int bit = game::MapFilterChosen(map);
    std::vector<std::string> names;
    std::vector<Ptr> stack = {map};
    for (int n = 0; !stack.empty() && n < 1000; n++) {
        Ptr w = stack.back();
        stack.pop_back();
        if (game::IsKindOf(w, "cWinLotBtn")) {
            Item it;
            it.widget = w;
            it.number = game::LotNumber(w);
            if (game::IsCommunityLot(it.number) && (game::LotFilterBits(w) & bit))
                names.push_back(LotPlaceName(it));
            continue;
        }
        for (Ptr c : game::Children(w)) stack.push_back(c);
    }
    if (!bit || names.empty()) return S(Str::LotsMarkedNone);
    std::string list;
    for (const std::string& name : names) list += (list.empty() ? "" : ", ") + name;
    return util::Format(S(Str::LotsMarked), list.c_str());
}

std::string AfterPress(Ptr root, Ptr pressed) {
    if (game::IsOutfitArrow(root, pressed)) return DescribeOutfit(root);
    if (Ptr map = game::TravelMap(root))
        for (const game::MapFilter& f : game::MapFilters(map))
            if (f.button == pressed) return DescribeMarkedLots(map);
    game::LookPart part;
    if (game::IsLookArrow(root, pressed, &part)) return DescribeLook(root, part);
    for (Ptr choice : game::LookControls(root))
        if (choice == pressed && game::IsKindOf(pressed, "cTSWinBtn") && game::ButtonChecked(pressed))
            return std::string(S(Str::Selected)) + ". " + DescribeLook(root, game::LookPart::Head) + ". " +
                   DescribeLook(root, game::LookPart::Body);
    return "";
}

std::string GameSpeedName(Ptr vc) {
    game::GameSpeed speed = game::ReadGameSpeed(vc);
    Ptr button = speed.paused ? game::PauseButton(vc) : game::SpeedButton(vc, speed.level);
    return button ? util::WithoutHotkey(game::Tooltip(button)) : "";
}

std::string Describe(const Item& it) {
    switch (it.role) {
    case Role::Text:
        return it.label;
    case Role::PieItem:  // "Hablar..., botón"
        return it.label + ", " + S(Str::RoleButton);
    case Role::Lot:
        return LotText(it);
    case Role::FamilyRow:
        return FamilyRowText(it);
    case Role::Motive:
        // The bar runs from -100 (empty, red) to 100 (full): spoken as percent full.
        return util::Format(S(Str::MotiveValue), it.label.c_str(), (game::MotiveValue(it.widget) + 100) / 2);
    case Role::QueuedAction:  // "Hablar, en cola, botón": pressing cancels it
        return util::Format(S(Str::QueuedAction), it.label.c_str()) + ", " + S(Str::RoleButton);
    case Role::TutorialIcon:
        return S(Str::TutorialIcon) + std::string(", ") + S(Str::RoleButton);
    case Role::Button:
        if (tutorial::IsStepHideButton(it)) return S(Str::TutorialHide) + std::string(", ") + S(Str::RoleButton);
        return ButtonText(it);
    case Role::BudgetRow:
        return BudgetRowText(it);
    case Role::Status:
        return StatusText(it);
    case Role::Mood:
        return MoodText(it);
    case Role::Relationship:
        return RelationshipText(it);
    case Role::Skill:
        return SkillText(it);
    case Role::JobInfo:
        return JobInfoText(it);
    case Role::Trait:
        return TraitText(it);
    case Role::HouseRating: {  // "Tamaño, 4 de 10"
        int ratings[game::kHouseRatingCount];
        if (!game::HouseRatings(ratings) || it.number < 0 || it.number >= game::kHouseRatingCount) return it.label;
        return util::Format(S(Str::SkillValue), it.label.c_str(), ratings[it.number]);
    }
    case Role::Info:
        return game::Caption(it.widget);
    case Role::PickerMember:
        return PickerMemberText(it);
    case Role::Spinner:  // "Detalle del terreno, selector, Alta"
        return (it.label.empty() ? std::string() : it.label + ", ") + S(Str::RolePicker) + ", " +
               util::CleanForSpeech(game::Caption(it.widget));
    case Role::Slider: {  // "Efectos, control deslizante, 8 de 10"
        game::SliderState s = game::ReadSlider(it.widget);
        return NameOf(it) + ", " + S(Str::RoleSlider) + ", " +
               util::Format(S(Str::ValueOfMax), s.value - s.min, s.max - s.min);
    }
    case Role::CheckBox:  // "Autocentrado, casilla de verificación, marcado"
        return NameOf(it) + ", " + S(Str::RoleCheckBox) + ", " +
               S(game::CheckBoxChecked(it.widget) ? Str::Checked : Str::NotChecked);
    case Role::SimPreview: {  // "vista del Sim: cabeza 2 de 137: dave. cuerpo ..."
        Ptr screen = game::Parent(it.widget);
        if (game::OutfitPreview(screen) == it.widget) return S(Str::SimPreview) + std::string(": ") + DescribeOutfit(screen);
        return S(Str::SimPreview) + std::string(": ") + DescribeLook(screen, game::LookPart::Head) + ". " +
               DescribeLook(screen, game::LookPart::Body);
    }
    case Role::TraitBar:  // "Pulcritud, control deslizante, 3 de 10"
        return it.label + ", " + S(Str::RoleSlider) + ", " + util::Format(S(Str::OutOfTen), game::TraitBarPoints(it.widget));
    case Role::TextField: {  // "Introduce apellido: luna, campo de texto"
        std::string text = game::TextFieldText(it.widget);
        return util::Format(S(Str::TextField), it.label.c_str(), text.empty() ? S(Str::Blank) : text.c_str());
    }
    case Role::Interest:  // "Música, 10 de 10"
        return util::Format(S(Str::SkillValue), it.label.c_str(), game::InterestPoints(it.widget));
    case Role::Product:  // "Silla todoterreno Nalghaus, $80, comodidad 2"
        return buy::ProductText(it.widget);
    case Role::Tab:
    case Role::FamilyMember:
        return ButtonText(it);
    }
    return it.label;
}

std::vector<std::string> DetailLines(const Item& it, const Group& g, int index) {
    std::vector<std::string> lines;
    if (it.role == Role::Product) lines = buy::ProductDetails(it.widget);
    else lines.push_back(Describe(it));
    std::string tip = util::CleanForSpeech(game::Tooltip(it.widget));
    if (!tip.empty() && lines.front().find(tip) == std::string::npos) lines.push_back(tip);
    lines.push_back(util::Format(S(Str::PositionInGroup), index + 1, (int)g.items.size(), GroupName(g).c_str()));
    return lines;
}

// Counts controls only: a title that names the group is not an item to the listener.
std::string GroupSummary(const Group& g) {
    int controls = 0;
    for (const Item& it : g.items) controls += it.role != Role::Text;
    if (controls == 0) controls = (int)g.items.size();
    std::string name = GroupName(g);
    return controls == 1 ? util::Format(S(Str::GroupSummaryOne), name.c_str())
                         : util::Format(S(Str::GroupSummary), name.c_str(), controls);
}

void ClearDescriptionCache() { g_houseCards.clear(); }

}  // namespace nav
