#pragma once

#include "bak/types.hpp"

#include <cmath>
#include <optional>

namespace BAK {
class Character;
class Party;
struct ShopStats;
}

namespace BAK::Haggle {

struct HaggleResult
{
    unsigned mDiscount;
    unsigned mDiscountPct;
};

std::optional<HaggleResult> TryHaggle(
    Party& party,
    ActiveCharIndex character,
    ShopStats& shop,
    ItemIndex item,
    int shopCurrentValue);

}
