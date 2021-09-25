#pragma once

#include "bak/encounter/gdsEntry.ipp"

namespace BAK::Encounter {

struct backgroundFile { static constexpr auto file = "DEF_BKGR.DAT"; };

using BackgroundFactory = GDSEntryFactory<backgroundFile>;

}
