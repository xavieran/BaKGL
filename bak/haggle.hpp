#pragma once

#include "bak/character.hpp"
#include "bak/party.hpp"
#include "bak/shop.hpp"

#include "com/random.hpp"

#include <algorithm>
#include <cmath>

namespace BAK::Haggle {

std::optional<unsigned> TryHaggle(
    Party& party,
    ActiveCharIndex character,
    ShopStats& shop,
    ItemIndex item,
    int shopCurrentValue);

}
