#pragma once

#include "bak/dialogTarget.hpp"

#include <vector>

namespace BAK::Encounters {

// Almost identical to combats but with 10 extra bytes...
class Trap
{
public:
    unsigned mTrapIndex;
    KeyTarget mEntryDialog;
};

class TrapFactory
{
public:
    static constexpr auto sFilename = "DEF_TRAP.DAT";

    TrapFactory();

    const Trap& Get(unsigned i) const;

private:
    void Load();
    std::vector<Trap> mTraps;
};


}
