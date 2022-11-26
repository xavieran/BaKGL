#pragma once

#include "com/strongType.hpp"

namespace BAK {

using Sovereigns = StrongType<unsigned, struct SovereignsTag>;
using Royals = StrongType<unsigned, struct RoyalsTag>;

static constexpr Royals sUnpurchaseablePrice{0xffffffff};

Sovereigns GetSovereigns(Royals);
Royals GetRoyals(Sovereigns sovereigns);
Royals GetRemainingRoyals(Royals);

std::string ToString(Royals);
std::string ToShopString(Royals);
std::string ToShopDialogString(Royals);

}
