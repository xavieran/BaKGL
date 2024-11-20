#pragma once

#include "bak/types.hpp"

#include <cmath>

namespace BAK {
class Character;
class Party;
struct ShopStats;
}

namespace BAK::Haggle {

std::optional<unsigned> TryHaggle(
    Party& party,
    ActiveCharIndex character,
    ShopStats& shop,
    ItemIndex item,
    int shopCurrentValue);

}
