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

enum class CombatantMoveType
{
    Stationary = 0,
    // Straight line movement
    // The two end points x and y must be met exactly for the combatant to turn around.
    // The combatant will continue in the direction of their heading until
    // they reach an endpoint at which point they will turn around 180
    // This didn't seem to work for me when combatants heading was not in 
    // a single direction
    StraightLine = 1,
    StraightLine2 = 2,

    // Combatant follows the road to the endpoints denoted
    FollowRoad = 3,
    FollowRoad2 = 4
};

// Combat with index 31 has a combatant with move type 2
// It has:
// pos: (7200, 34115, h0)
// minX, 7200 maxX 15200
// unknown0 15200, 7200
// minY 35715, maxY 35715
// unknown1
// 34115, 34115
// 0xffff

struct CombatantData
{
    CombatantData(
        std::uint16_t monster,
        std::uint16_t movementType,
        GamePositionAndHeading location,
        glm::ivec2 min,
        glm::ivec2 max)
    :
        mMonster{monster},
        mMovementType{movementType},
        mLocation{location},
        mMin{min},
        mMax{max}
    {}

    std::uint16_t mMonster;
    std::uint16_t mMovementType;
    GamePositionAndHeading mLocation;
    glm::ivec2 mMin;
    glm::ivec2 mMax;
    // path...
};

std::ostream& operator<<(std::ostream&, const CombatantData&);

// From REQ_TE12.DAT:
// Combat Situation
// Objects
// Enter Dialogue
// Scout Dialogue
// Ambush / Regular
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
        std::vector<CombatantData> combatants,
        std::uint16_t unknown,
        bool isAmbush)
    :
        mCombatIndex{combatIndex},
        mEntryDialog{entryDialog},
        mScoutDialog{scoutDialog},
        mTrap{trap},
        mNorthRetreat{northRetreat},
        mWestRetreat{westRetreat},
        mSouthRetreat{southRetreat},
        mEastRetreat{eastRetreat},
        mCombatants{combatants},
        mUnknown{unknown},
        mIsAmbush{isAmbush}
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
    std::uint16_t mUnknown;
    // ambushes are not visible, but are scoutable
    // not ambushes are visible, but not scoutable
    bool mIsAmbush;
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
