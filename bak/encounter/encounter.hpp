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
#include <vector>

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
        glm::uvec2 location,
        glm::uvec2 dims,
        glm::uvec2 tile,
        unsigned tileIndex,
        unsigned saveAddress,
        unsigned saveAddress2,
        unsigned saveAddress3,
        std::uint8_t unknown0,
        std::uint8_t unknown1,
        std::uint8_t unknown2,
        std::uint16_t unknown3)
    :
        mEncounter{encounter},
        mIndex{index},
        mLocation{location},
        mDimensions{dims},
        mTile{tile},
        mTileIndex{tileIndex},
        mSaveAddress{saveAddress},
        mSaveAddress2{saveAddress2},
        mSaveAddress3{saveAddress3},
        mUnknown0{unknown0},
        mUnknown1{unknown1},
        mUnknown2{unknown2},
        mUnknown3{unknown3}
    {}

    const auto& GetEncounter() const { return mEncounter; }
    // This is specifically the index of the encounter in the DAT file
    // NOT the index of the encounter type in its table (DEF_BLOC etc.)
    auto GetIndex() const { return mIndex; }
    auto GetSaveAddress() const { return mSaveAddress; }
    auto GetTile() const { return mTile; }
    auto GetTileIndex() const { return mTileIndex; }

    auto GetLocation() const
    {
        return ToGlCoord<float>(mLocation);
    }

    auto GetDims() const
    {
        return mDimensions;
    }

    EncounterT mEncounter;
    EncounterIndex mIndex;
    glm::uvec2 mLocation;
    glm::uvec2 mDimensions;
    glm::uvec2 mTile;
    unsigned mTileIndex;

    // Place in the save file that is checked
    // by this encounter to see if it has
    // already been encountered
    unsigned mSaveAddress;
    unsigned mSaveAddress2;
    unsigned mSaveAddress3;
    std::uint8_t mUnknown0;
    std::uint8_t  mUnknown1;
    std::uint8_t  mUnknown2;
    std::uint16_t mUnknown3;
    friend std::ostream& operator<<(std::ostream&, const Encounter&);
};

std::ostream& operator<<(std::ostream&, const Encounter&);

class EncounterStore
{
public:
    EncounterStore(
        const EncounterFactory&,
        FileBuffer& fb,
        glm::uvec2 tile,
        unsigned tileIndex);
    
    const std::vector<Encounter>& GetEncounters(
        Chapter chapter) const;
    
private:
    std::vector<
        std::vector<Encounter>> mChapters;
};

std::vector<Encounter> LoadEncounters(
    const EncounterFactory&,
    FileBuffer& fb,
    Chapter chapter,
    glm::uvec2 tile,
    unsigned tileIndex);

}
