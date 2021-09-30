#pragma once

#include "bak/coordinates.hpp"
#include "bak/dialogTarget.hpp"

#include <optional>
#include <vector>

namespace BAK::Encounter {

// From REQ_TE12.DAT:
// Combat Situation
// Objects
// Enter Dialogue
// Scout Dialogue
// Ambush
// North Retreat
// West Retreat
// South Retreat
// East Retreat

class Combat
{
public:
    unsigned mCombatIndex;
    KeyTarget mEntryDialog;
    KeyTarget mScoutDialog;

    std::optional<GamePositionAndHeading> mTrap;
    GamePositionAndHeading mNorthRetreat;
    GamePositionAndHeading mWestRetreat;
    GamePositionAndHeading mSouthRetreat;
    GamePositionAndHeading mEastRetreat;

    std::vector<unsigned> mCombatants;
    //bool mVisible;
};

std::ostream& operator<<(std::ostream&, const Combat&);

template <bool isTrap>
class GenericCombatFactory
{
public:
    static constexpr auto sCombatFilename = "DEF_COMB.DAT";
    static constexpr auto sTrapFilename   = "DEF_TRAP.DAT";

    GenericCombatFactory();

    const Combat& Get(unsigned i) const;

private:
    void Load();
    std::vector<Combat> mCombats;
};

using CombatFactory = GenericCombatFactory<false>;

}
