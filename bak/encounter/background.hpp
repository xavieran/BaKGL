#pragma once

#include "bak/encounter/gdsEntry.hpp"

namespace BAK::Encounter {

struct BackgroundFile
{
    static constexpr auto file = "DEF_BKGR.DAT";
};

using BackgroundFactory = GDSEntryFactory<BackgroundFile>;

}
