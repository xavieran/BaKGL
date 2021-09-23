#include "bak/encounter.hpp"

#include "graphics/glm.hpp"
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

namespace BAK {

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
    os << e.mEncounterType << " i: " << e.mEncounterIndex << " loc: "
        << e.mLocation << " tile: " << e.mTile << std::hex << " savePtr: ("
        << e.mSaveAddress << " " << e.mSaveAddress2 
        << ") Unknown: " << e.mUnknown0 << " "
        << e.mUnknown1 << " " << e.mUnknown2 << std::dec;
    return os;
}

std::vector<Encounter> LoadEncounters(
    FileBuffer& fb,
    unsigned chapter,
    glm::vec<2, unsigned> tile)
{
    std::vector<Encounter> encounters{};
    // Ideally load all encounters... each chapter can be part of the
    // encounter type and they can be filtered later
    fb.Seek((chapter - 1) * 192);
    unsigned numberOfEncounters = fb.GetUint16LE();

    encounters.reserve(numberOfEncounters);
    
    for (unsigned i = 0; i < numberOfEncounters; i++)
    {
        auto loc = fb.Tell();
        auto encounterType = static_cast<BAK::EncounterType>(fb.GetUint16LE());
        
        const unsigned left   = fb.GetUint8();
        const unsigned top    = fb.GetUint8();
        const unsigned right  = fb.GetUint8();
        const unsigned bottom = fb.GetUint8();
        auto l = tile.x * 64000 + left * 1600;
        auto r = tile.x * 64000 + right * 1600;
        auto t = tile.y * 64000 + bottom * 1600;
        auto b = tile.y * 64000 + bottom * 1600;
        auto xLoc = (l + r) / 2;
        auto yLoc = (t + b) / 2;
        unsigned encounterIndex = fb.GetUint16LE();
        // Don't know
        const auto unknown0 = fb.GetUint32LE();
        const auto unknown1 = fb.GetUint8();
        unsigned saveAddr = fb.GetUint16LE();
        unsigned saveAddr2 = fb.GetUint16LE();
        const auto unknown2 = fb.GetUint16LE();

        std::cout << "Loaded encounter: " << tile << " lc: " << xLoc << "," << yLoc
            << " @ 0x" << std::hex << loc
            << std::dec << " type: " << encounterType << " index: " << encounterIndex
            << " saveAddr: 0x" << std::hex << saveAddr << std::dec << std::endl;

        encounters.emplace_back(
            encounterType,
            encounterIndex,
            glm::vec<2, int>{xLoc, yLoc},
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
