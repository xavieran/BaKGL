#pragma once

#include "bak/constants.hpp"

#include "bak/encounter/background.hpp"
#include "bak/encounter/block.hpp"
#include "bak/encounter/combat.hpp"
#include "bak/encounter/dialog.hpp"
#include "bak/encounter/disable.hpp"
#include "bak/encounter/enable.hpp"
#include "bak/encounter/town.hpp"
#include "bak/encounter/trap.hpp"
#include "bak/encounter/zone.hpp"

#include "com/strongType.hpp"

#include <glm/glm.hpp>

#include <ostream>
#include <string_view>
#include <variant>

namespace BAK::Encounter {

using EncounterIndex = StrongType<unsigned, struct EncounterIndexTag>;

// These are enumerated in "LIST_TYP.DAT"
enum class EncounterType : std::uint16_t
{
    Background = 0, // DEF_BKGR.DAT?? teleport??
    Combat = 1,  // DEF_COMB.DAT
    Comment = 2,  // no file exists, but would be DEF_COMM.DAT
    Dialog = 3,  // DEF_DIAL.DAT
    Health = 4,  // no file exists, but would be DEF_HEAL.DAT
    Sound = 5,  // DEF_SOUN.DAT
    Town = 6,  // DEF_TOWN.DAT
    Trap = 7,  // DEF_TRAP.DAT
    Zone = 8,  // DEF_ZONE.DAT
    Disable = 9,  // DEF_DISA.DAT
    Enable = 0xa,  // DEF_ENAB.DAT
    Block = 0xb  // DEF_BLOC.DAT
};

std::string_view ToString(EncounterType t);
std::ostream& operator<<(std::ostream& os, EncounterType e);

using EncounterT = std::variant<
    GDSEntry,
    Block,
    Combat,
    Dialog,
    EventFlag,
    Zone>;

std::ostream& operator<<(std::ostream& os, const EncounterT&);
std::string_view ToString(const EncounterT&);

class EncounterFactory
{
public:
    EncounterT MakeEncounter(
        EncounterType,
        unsigned,
        glm::uvec2 tile) const;

private:
    BackgroundFactory mBackgrounds;
    BlockFactory mBlocks;
    CombatFactory mCombats;
    DialogFactory mDialogs;
    DisableFactory mDisables;
    EnableFactory mEnables;
    TownFactory mTowns;
    TrapFactory mTraps;
    ZoneFactory mZones;
};

class Encounter
{
public:
    Encounter(
        EncounterT encounter,
        EncounterIndex index,
        std::uint16_t encounterTableIndex,
        glm::uvec2 topLeft,
        glm::uvec2 bottomRight,
        glm::uvec2 center,
        glm::uvec2 location,
        glm::uvec2 dims,
        glm::uvec2 tile,
        unsigned tileIndex,
        unsigned tileCombatIndex,
        unsigned requiredState,
        unsigned inhibitState,
        unsigned completionState,
        std::uint8_t unknown0,
        std::uint8_t unknown1,
        std::uint8_t chapterFlag,
        std::uint16_t repeatable)
    :
        mEncounter{encounter},
        mIndex{index},
        mEncounterTableIndex{encounterTableIndex},
        mTopLeft{topLeft},
        mBottomRight{bottomRight},
        mCenter{(topLeft + bottomRight) / 2u},
        mLocation{location},
        mDimensions{dims},
        mTile{tile},
        mTileIndex{tileIndex},
        mTileCombatIndex{tileCombatIndex},
        mRequiredState{requiredState},
        mInhibitState{inhibitState},
        mCompletionState{completionState},
        mUnknown0{unknown0},
        mUnknown1{unknown1},
        mChapterFlag{chapterFlag},
        mRepeatable{repeatable}
    {}

    const auto& GetEncounter() const { return mEncounter; }
    // This is specifically the index of the encounter in the DAT file
    // NOT the index of the encounter type in its table (DEF_BLOC etc.)
    auto GetIndex() const { return mIndex; }
    auto GetRequiredState() const { return mRequiredState; }
    auto GetTile() const { return mTile; }
    auto GetTileIndex() const { return mTileIndex; }
    auto GetTileCombatIndex() const { return mTileCombatIndex; }

    auto GetLocation() const
    {
        return ToGlCoord<float>(mLocation);
    }

    auto GetDims() const
    {
        return mDimensions;
    }

    auto GetCenter() const
    {
        return mCenter;
    }

    EncounterT mEncounter;
    EncounterIndex mIndex;
    std::uint16_t mEncounterTableIndex;
    glm::uvec2 mTopLeft;
    glm::uvec2 mBottomRight;
    glm::uvec2 mCenter;
    glm::uvec2 mLocation;
    glm::uvec2 mDimensions;
    glm::uvec2 mTile;
    unsigned mTileIndex;
    unsigned mTileCombatIndex;

    unsigned mRequiredState;
    unsigned mInhibitState;
    unsigned mCompletionState;
    std::uint8_t mUnknown0;
    std::uint8_t  mUnknown1;
    std::uint8_t  mChapterFlag;
    std::uint16_t mRepeatable;
    friend std::ostream& operator<<(std::ostream&, const Encounter&);
};

std::ostream& operator<<(std::ostream&, const Encounter&);

}
