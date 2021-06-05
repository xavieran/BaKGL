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
        
        int xLoc = fb.GetSint16LE();
        int yLoc = fb.GetSint16LE();
        unsigned encounterIndex = fb.GetUint16LE();
        // Don't know
        fb.GetUint16LE();
        fb.GetUint16LE();
        fb.GetUint8();
        unsigned saveAddr = fb.GetUint16LE();
        fb.GetUint16LE();
        fb.GetUint16LE();

        std::cout << "Loaded encounter: " << tile << " lc: " << xLoc << "," << yLoc
            << " @ 0x" << std::hex << loc
            << std::dec << " type: " << encounterType << " index: " << encounterIndex
            << " saveAddr: 0x" << std::hex << saveAddr << std::dec << std::endl;

        encounters.emplace_back(
            encounterType,
            encounterIndex,
            glm::vec<2, int>{xLoc, yLoc},
            tile,
            saveAddr);
    }

    return encounters;
}

}
