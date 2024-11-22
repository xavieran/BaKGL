#pragma once

#include "bak/types.hpp"

namespace BAK {

static constexpr Royals sUnpurchaseablePrice{0xffffffff};

Sovereigns GetSovereigns(Royals);
Royals GetRoyals(Sovereigns sovereigns);
Royals GetRemainingRoyals(Royals);

std::string ToString(Royals);
std::string ToShopString(Royals);
std::string ToShopDialogString(Royals);

}
