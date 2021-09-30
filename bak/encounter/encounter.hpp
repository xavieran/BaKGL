#pragma once

#include "bak/constants.hpp"

#include "bak/encounter/gdsEntry.hpp"
#include "bak/encounter/background.hpp"
#include "bak/encounter/block.hpp"
#include "bak/encounter/combat.hpp"
#include "bak/encounter/dialog.hpp"
#include "bak/encounter/disable.hpp"
#include "bak/encounter/enable.hpp"
#include "bak/encounter/sound.hpp"
#include "bak/encounter/town.hpp"
#include "bak/encounter/trap.hpp"
#include "bak/encounter/zone.hpp"

#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

#include <iostream>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

namespace BAK::Encounter {

using EncounterIndex = unsigned;
// These are enumerated in "LIST_TYP.DAT"
enum class EncounterType : std::uint16_t
{
    Background = 0, // DEF_BKGR.DAT?? teleport??
    Combat = 1,  // DEF_COMB.DAT
    Comment = 2,  // ???
    Dialog = 3,  // DEF_DIAL.DAT
    Health = 4,  // ???
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
        EncounterIndex,
        glm::vec<2, unsigned> tile) const;

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
        glm::vec<2, unsigned> location,
        glm::vec<2, unsigned> dims,
        glm::vec<2, unsigned> tile,
        unsigned saveAddress,
        unsigned saveAddress2,
        unsigned saveAddress3,
        std::uint8_t unknown0,
        std::uint16_t unknown1,
        std::uint16_t unknown2)
    :
        mEncounter{encounter},
        mLocation{location},
        mDimensions{dims},
        mTile{tile},
        mSaveAddress{saveAddress},
        mSaveAddress2{saveAddress2},
        mSaveAddress3{saveAddress3},
        mUnknown0{unknown0},
        mUnknown1{unknown1},
        mUnknown2{unknown2}
    {}

    const auto& GetEncounter() const { return mEncounter; }
    auto GetSaveAddress() const { return mSaveAddress; }
    auto GetTile() const { return mTile; }

    auto GetLocation() const
    {
        return glm::vec3{
            static_cast<float>(mLocation.x),
            0.0f,
            -(static_cast<float>(mLocation.y))};

    }

    auto GetDims() const
    {
        return mDimensions;
    }

    EncounterT mEncounter;
    glm::vec<2, unsigned> mLocation;
    glm::vec<2, unsigned> mDimensions;
    glm::vec<2, unsigned> mTile;

    // Place in the save file that is checked
    // by this encounter to see if it has
    // already been encountered
    unsigned mSaveAddress;
    unsigned mSaveAddress2;
    unsigned mSaveAddress3;
    std::uint8_t mUnknown0;
    std::uint16_t  mUnknown1;
    std::uint16_t mUnknown2;
    friend std::ostream& operator<<(std::ostream&, const Encounter&);
};

std::ostream& operator<<(std::ostream&, const Encounter&);

std::vector<Encounter> LoadEncounters(
    const EncounterFactory&,
    FileBuffer& fb,
    unsigned chapter,
    glm::vec<2, unsigned> tile);

}
