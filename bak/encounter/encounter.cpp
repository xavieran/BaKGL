#include "bak/encounter/encounter.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include <iostream>
#include <string_view>
#include <vector>

namespace BAK::Encounter {

std::string_view ToString(EncounterType t)
{
    switch (t)
    {
    case EncounterType::Background: return "background";
    case EncounterType::Combat: return "combat";
    case EncounterType::Comment: return "comment";
    case EncounterType::Dialog: return "dialog";
    case EncounterType::Health: return "health";
    case EncounterType::Sound: return "sound";
    case EncounterType::Town: return "town";
    case EncounterType::Trap: return "trap";
    case EncounterType::Zone: return "zone";
    case EncounterType::Disable: return "disable";
    case EncounterType::Enable: return "enable";
    case EncounterType::Block: return "block";
    default: return "unknown";
    }
}

std::string_view ToString(const EncounterT& encounter)
{
    return std::visit(
        overloaded{
            [](const GDSEntry&){ return "GDSEntry"; },
            [](const Block&){ return "Block"; },
            [](const Combat&){ return "Combat"; },
            [](const Dialog&){ return "Dialog"; },
            [](const EventFlag&){ return "EventFlag"; },
            [](const Zone&){ return "Zone"; }
        },
        encounter);
}
 
std::ostream& operator<<(std::ostream& os, EncounterType e)
{
    return os << ToString(e);
}

std::ostream& operator<<(std::ostream& os, const EncounterT& encounter)
{
    std::visit([&os](const auto& e){ os << e; }, encounter);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Encounter& e)
{
    os << "Encounter { index: " << e.mIndex
        << " ETI: " << e.mEncounterTableIndex
        << " dims: " << e.mDimensions
        << " worldLocation: " << e.mLocation
        << " TL: " << e.mTopLeft
        << " BR: " << e.mBottomRight
        << " tile: " << e.mTile 
        << " tileIndex: " << e.mTileIndex 
        << std::hex << " savePtr: ("
        << e.mSaveAddress << ", " << e.mSaveAddress2 << ", "
        << e.mSaveAddress3
        << ") Unknown [" << +e.mUnknown0 << ","
        << +e.mUnknown1 << "," << +e.mUnknown2 << "," 
        << +e.mUnknown3 << "]" << std::dec
        << "{" << e.mEncounter << "}}";
    return os;
}

EncounterT EncounterFactory::MakeEncounter(
    EncounterType eType,
    unsigned encounterIndex,
    glm::uvec2 tile) const
{
    switch (eType)
    {
    case EncounterType::Background:
        return mBackgrounds.Get(encounterIndex, tile);
    case EncounterType::Combat:
        {
            auto combat = mCombats.Get(encounterIndex);
            const auto tilePos = tile * static_cast<unsigned>(64000);
            combat.mNorthRetreat.mPosition += tilePos;
            combat.mSouthRetreat.mPosition += tilePos;
            combat.mWestRetreat.mPosition += tilePos;
            combat.mEastRetreat.mPosition += tilePos;
            for (auto& combatant : combat.mCombatants)
            {
                combatant.mLocation.mPosition += tilePos;
            }
            return combat;
        }
    case EncounterType::Comment:
        throw std::runtime_error("Can't make COMMENT encounters");
    case EncounterType::Dialog:
        return mDialogs.Get(encounterIndex);
    case EncounterType::Health:
        throw std::runtime_error("Can't make HEALTH encounters");
    case EncounterType::Sound:
        throw std::runtime_error("Can't make SOUND encounters");
    case EncounterType::Town:
        return mTowns.Get(encounterIndex, tile);
    case EncounterType::Trap:
        {
            auto combat = mTraps.Get(encounterIndex);
            const auto tilePos = tile * static_cast<unsigned>(64000);
            combat.mNorthRetreat.mPosition += tilePos;
            combat.mSouthRetreat.mPosition += tilePos;
            combat.mWestRetreat.mPosition += tilePos;
            combat.mEastRetreat.mPosition += tilePos;
            for (auto& combatant : combat.mCombatants)
            {
                combatant.mLocation.mPosition += tilePos;
            }
            return combat;
        }
    case EncounterType::Zone:
        return mZones.Get(encounterIndex);
    case EncounterType::Disable:
        return mDisables.Get(encounterIndex);
    case EncounterType::Enable:
        return mEnables.Get(encounterIndex);
    case EncounterType::Block:
        return mBlocks.Get(encounterIndex);
    default:
        throw std::runtime_error("Can't make UNKNOWN encounters");
    }
}



}
