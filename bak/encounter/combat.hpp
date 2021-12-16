#pragma once

#include "bak/coordinates.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/types.hpp"

#include <optional>
#include <vector>

namespace BAK::Encounter {

// From REQ_TE13.DAT:
// Type (Monster type)
// Movement Style
// Where (location)
// Width
// Height
// Radius
// Road End 1
// Road End 2

struct CombatantData
{
    CombatantData(
        std::uint16_t monster,
        std::uint16_t movementType,
        GamePositionAndHeading location)
    :
        mMonster{monster},
        mMovementType{movementType},
        mLocation{location}
    {}

    std::uint16_t mMonster;
    std::uint16_t mMovementType;
    GamePositionAndHeading mLocation;
    // path...
};

std::ostream& operator<<(std::ostream&, const CombatantData&);

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
    Combat(
        unsigned combatIndex,
        KeyTarget entryDialog,
        KeyTarget scoutDialog,
        std::optional<GamePositionAndHeading> trap,
        GamePositionAndHeading northRetreat,
        GamePositionAndHeading westRetreat,
        GamePositionAndHeading southRetreat,
        GamePositionAndHeading eastRetreat,
        std::vector<CombatantData> combatants)
    :
        mCombatIndex{combatIndex},
        mEntryDialog{entryDialog},
        mScoutDialog{scoutDialog},
        mTrap{trap},
        mNorthRetreat{northRetreat},
        mWestRetreat{westRetreat},
        mSouthRetreat{southRetreat},
        mEastRetreat{eastRetreat},
        mCombatants{combatants}
    {}

    unsigned mCombatIndex;
    KeyTarget mEntryDialog;
    KeyTarget mScoutDialog;

    std::optional<GamePositionAndHeading> mTrap;
    GamePositionAndHeading mNorthRetreat;
    GamePositionAndHeading mWestRetreat;
    GamePositionAndHeading mSouthRetreat;
    GamePositionAndHeading mEastRetreat;

    std::vector<CombatantData> mCombatants;
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
