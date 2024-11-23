#pragma once

#include "bak/encounter/gdsEntry.hpp"

namespace BAK::Encounter {

struct TownFile
{
    static constexpr auto file = "DEF_TOWN.DAT";
};

using TownFactory = GDSEntryFactory<TownFile>;

}
