#include "catalog_find.h"

#include "../game/buy.h"
#include "../speech/speech.h"
#include "../util/log.h"
#include "../util/strings.h"
#include "../util/text.h"
#include "buy_mode.h"

namespace nav::catalog_find {
namespace {

struct State {
    game::Ptr catalog = 0;              // the Catalog the products belong to
    std::vector<game::Ptr> products;    // the matches, best first
    std::string label;                  // the text as typed
};

State g;

// A product of the list, 0 when the list is stale (the lot changed and the catalog with it).
game::Ptr ProductAt(int number) {
    if (g.catalog != game::BuyCatalog()) g = State{};
    return number >= 0 && number < (int)g.products.size() ? g.products[number] : 0;
}

}  // namespace

int SetQuery(const std::string& utf8) {
    std::string folded = util::FoldForSearch(utf8);
    if (folded.empty()) return 0;
    std::vector<game::Ptr> ranked[3];
    for (game::Ptr product : game::BuyCatalogProducts()) {
        game::ProductInfo p;
        if (!game::ReadProduct(product, &p)) continue;
        std::string name = util::FoldForSearch(p.name);
        size_t at = name.find(folded);
        if (at == std::string::npos) continue;
        int rank = at == 0 ? 0 : name.find(" " + folded) != std::string::npos ? 1 : 2;
        ranked[rank].push_back(product);
    }
    std::vector<game::Ptr> all;
    for (auto& r : ranked) all.insert(all.end(), r.begin(), r.end());
    if (all.empty()) return 0;
    g.catalog = game::BuyCatalog();
    g.products = std::move(all);
    g.label = utf8;
    LogVerbose("catalog find '%s': %d products", utf8.c_str(), (int)g.products.size());
    return (int)g.products.size();
}

void Clear() { g = State{}; }

bool Shown() {
    if (g.products.empty()) return false;
    if (g.catalog != game::BuyCatalog()) {
        g = State{};
        return false;
    }
    return game::BuyCatalogShown();
}

Group ResultsGroup() {
    Group group{kCatalogFindGroup, {}, {}};
    if (!Shown()) return group;
    for (int i = 0; i < (int)g.products.size(); i++) {
        game::ProductInfo p;
        if (!game::ReadProduct(g.products[i], &p)) continue;
        Item it;
        it.widget = kCatalogFindGroup;
        it.role = Role::CatalogResult;
        it.number = i;
        it.label = p.name;
        group.items.push_back(std::move(it));
    }
    return group;
}

std::string GroupName() { return util::Format(S(Str::GroupCatalogFind), g.label.c_str()); }

std::string Describe(int number) {
    game::Ptr product = ProductAt(number);
    return product ? buy::FoundProductText(product) : S(Str::Unavailable);
}

std::vector<std::string> Details(int number) {
    game::Ptr product = ProductAt(number);
    return product ? buy::FoundProductDetails(product) : std::vector<std::string>{S(Str::Unavailable)};
}

void Choose(int number) {
    game::Ptr product = ProductAt(number);
    game::ProductInfo p;
    if (!product || !game::ReadProduct(product, &p)) {
        speech::Say(S(Str::Unavailable));
        return;
    }
    switch (game::ChooseProduct(product)) {
    case game::Choice::Chosen: buy::Renew(); break;  // buy mode says the spots once it holds it
    case game::Choice::CannotAfford: speech::Say(p.name + ", " + S(Str::CannotAfford)); break;
    case game::Choice::Failed: speech::Say(S(Str::Unavailable)); break;
    }
}

}  // namespace nav::catalog_find
