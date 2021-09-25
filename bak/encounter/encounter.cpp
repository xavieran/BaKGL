#include "bak/encounter/encounter.hpp"

#include "com/logger.hpp"

#include "graphics/glm.hpp"
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

namespace BAK::Encounter {

std::string EncounterTypeToString(EncounterType t)
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

std::ostream& operator<<(std::ostream& os, EncounterType e)
{
    return os << EncounterTypeToString(e);
}

std::ostream& operator<<(std::ostream& os, const Encounter& e)
{
    os << e.mEncounterType 
        << " i: " << e.mEncounterIndex << " loc: " << e.mLocation 
        << " dims: " << e.mDimensions << " tile: " << e.mTile 
        << std::hex << " savePtr: ("
        << e.mSaveAddress << " " << e.mSaveAddress2 
        << ") Unknown: " << e.mUnknown0 << " "
        << e.mUnknown1 << " " << e.mUnknown2 << std::dec;
    return os;
}

unsigned TileOffsetToWorldLocation(unsigned tile, std::uint8_t offset)
{
    return tile * BAK::gTileSize + sOffsetScale * offset;
}

std::pair<
    glm::vec<2, unsigned>,
    glm::vec<2, unsigned>>
CalculateLocationAndDims(
    glm::vec<2, unsigned> tile,
    std::uint8_t l,
    std::uint8_t t,
    std::uint8_t r,
    std::uint8_t b)
{
    // Reminder - BAK coordinates origin is at bottom left
    // and x and y grow positive
    const auto left   = TileOffsetToWorldLocation(tile.x, l);
    const auto right  = TileOffsetToWorldLocation(tile.x, r);
    const auto top    = TileOffsetToWorldLocation(tile.y, t);
    const auto bottom = TileOffsetToWorldLocation(tile.y, b);
    // Give them some thickness
    const auto width = right == left
        ? sOffsetScale
        : right - left;
    const auto height = top == bottom
        ? sOffsetScale
        : top - bottom;

    const auto location = glm::vec<2, unsigned>{
        left + width / 2,
        bottom + height / 2};
    const auto dimensions = glm::vec<2, unsigned>{width, height};

    return std::make_pair(location, dimensions);
}


std::vector<Encounter> LoadEncounters(
    FileBuffer& fb,
    unsigned chapter,
    glm::vec<2, unsigned> tile)
{
    const auto& logger = Logging::LogState::GetLogger("LoadEncounter");
    std::vector<Encounter> encounters{};
    // Ideally load all encounters... each chapter can be part of the
    // encounter type and they can be filtered later
    fb.Seek((chapter - 1) * 192);
    unsigned numberOfEncounters = fb.GetUint16LE();

    encounters.reserve(numberOfEncounters);
    
    for (unsigned i = 0; i < numberOfEncounters; i++)
    {
        auto loc = fb.Tell();
        auto encounterType = static_cast<EncounterType>(fb.GetUint16LE());

        
        const unsigned left   = fb.GetUint8();
        const unsigned top    = fb.GetUint8();
        const unsigned right  = fb.GetUint8();
        const unsigned bottom = fb.GetUint8();

        const auto& [location, dimensions] = CalculateLocationAndDims(
            tile,
            left,
            top,
            right,
            bottom);

        const unsigned encounterIndex = fb.GetUint16LE();
        // Don't know
        const auto unknown0 = fb.GetUint32LE();
        const auto unknown1 = fb.GetUint8();
        const unsigned saveAddr = fb.GetUint16LE();
        const unsigned saveAddr2 = fb.GetUint16LE();
        const auto unknown2 = fb.GetUint16LE();

        logger.Debug() << "Loaded encounter: " << tile << " loc: " << location
            << " dims: " << dimensions << " @ 0x" << std::hex << loc
            << std::dec << " type: " << encounterType << " index: " << encounterIndex
            << " saveAddr: 0x" << std::hex << saveAddr << std::dec << std::endl;

        encounters.emplace_back(
            encounterType,
            encounterIndex,
            location,
            dimensions,
            tile,
            saveAddr,
            saveAddr2,
            unknown0,
            unknown1,
            unknown2);
    }

    return encounters;
}

}
