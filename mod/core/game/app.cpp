#include "app.h"

#include "addresses.h"
#include "memory.h"

namespace game {

int LanguageNumber() {
    Ptr root = Rd<Ptr>(Va(addr::kSimRoot));
    return root ? Rd<uint8_t>(root + addr::kLanguage) : 0;
}

}  // namespace game
