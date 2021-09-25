#pragma once

#include "bak/encounter/gdsEntry.ipp"

namespace BAK::Encounter {

struct townFile { static constexpr auto file = "DEF_TOWN.DAT"; };

using TownFactory = GDSEntryFactory<townFile>;

}
