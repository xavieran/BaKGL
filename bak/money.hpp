#pragma once

#include "com/strongType.hpp"

namespace BAK {

using Sovereigns = StrongType<unsigned, struct SovereignsTag>;
using Royals = StrongType<unsigned, struct RoyalsTag>;

Sovereigns GetSovereigns(Royals);
Royals GetRoyals(Sovereigns sovereigns);
Royals GetRemainingRoyals(Royals);

}
