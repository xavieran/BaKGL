#include "bak/encounter/encounterStore.hpp"

#include "bak/encounter/encounter.hpp"
#include "bak/file/fileBuffer.hpp"

#include "com/logger.hpp"


namespace BAK::Encounter {
std::pair<
    glm::uvec2,
    glm::uvec2>
CalculateLocationAndDims(
    glm::uvec2 tile,
    std::uint8_t l,
    std::uint8_t t,
    std::uint8_t r,
    std::uint8_t b)
{
    // Reminder - BAK coordinates origin is at bottom left
    // and x and y grow positive
    const auto topLeft = MakeGamePositionFromTileAndCell(
        tile, glm::vec<2, std::uint8_t>{l, t});
    const auto bottomRight = MakeGamePositionFromTileAndCell(
        tile, glm::vec<2, std::uint8_t>{r, b});
    // Give them some thickness
    const auto left = topLeft.x;
    const auto top = topLeft.y;
    const auto right = bottomRight.x;
    const auto bottom = bottomRight.y;
    ASSERT(right >= left && top >= bottom);
    const auto width = right == left
        ? gCellSize
        : right - left;
    const auto height = top == bottom
        ? gCellSize
        : top - bottom;

    // This is just not quite right... 
    // not sure if I am rendering items at the wrong place 
    // or if this is incorrect. According to the assembly
    // this 800 offset shouldn't be required...
    const auto xOffset = gCellSize / 2;
    const auto yOffset = xOffset;
    const auto location = GamePosition{
        left + (width / 2) - xOffset, 
        bottom + (height / 2) - yOffset};
    const auto dimensions = glm::uvec2{
        width, height};

    return std::make_pair(location, dimensions);
}

std::vector<Encounter> LoadEncounters(
    const EncounterFactory& ef,
    FileBuffer& fb,
    Chapter chapter,
    glm::uvec2 tile,
    unsigned tileIndex)
{
    const auto& logger = Logging::LogState::GetLogger("LoadEncounter");
    std::vector<Encounter> encounters{};
    // Ideally load all encounters... each chapter can be part of the
    // encounter type and they can be filtered later
    constexpr auto encounterEntrySize = 0x13;
    constexpr auto maxEncounters = 0xa;
    // + 2 for the count of encounters
    fb.Seek((chapter.mValue - 1) * (encounterEntrySize * maxEncounters + 2));
    unsigned numberOfEncounters = fb.GetUint16LE();

    encounters.reserve(numberOfEncounters);
    
    logger.Debug() << "Loading encounters for chapter: " << chapter.mValue << " encounters: " << numberOfEncounters << "\n";
    for (unsigned i = 0; i < numberOfEncounters; i++)
    {
        auto loc = fb.Tell();
        auto encounterType = static_cast<EncounterType>(fb.GetUint16LE());
        
        const unsigned left   = fb.GetUint8();
        const unsigned top    = fb.GetUint8();
        const unsigned right  = fb.GetUint8();
        const unsigned bottom = fb.GetUint8();
        const auto topLeft = glm::uvec2{left, top};
        const auto bottomRight = glm::uvec2{right, bottom};

        const auto& [location, dimensions] = CalculateLocationAndDims(
            tile,
            left,
            top,
            right,
            bottom);

        const unsigned encounterTableIndex = fb.GetUint16LE();
        // Don't know
        const auto unknown0 = fb.GetUint8();
        const auto unknown1 = fb.GetUint8();
        const auto unknown2 = fb.GetUint8();
        const auto saveAddr = fb.GetUint16LE();
        const unsigned saveAddr2 = fb.GetUint16LE();
        const unsigned saveAddr3 = fb.GetUint16LE();
        const auto unknown3 = fb.GetUint16LE();

        logger.Debug() << "Loaded encounter: " << tile << " loc: " << location
            << " dims: " << dimensions << " @ 0x" << std::hex << loc
            << std::dec << " type: " << encounterType << " index: " << encounterTableIndex
            << " saveAddr: 0x" << std::hex << saveAddr << ", " << saveAddr2 << ", "
            << saveAddr3 << std::dec << "\n";
        encounters.emplace_back(
            ef.MakeEncounter(
                encounterType,
                encounterTableIndex,
                tile),
            EncounterIndex{i},
            topLeft,
            bottomRight,
            location,
            dimensions,
            tile,
            tileIndex,
            saveAddr,
            saveAddr2,
            saveAddr3,
            unknown0,
            unknown1,
            unknown2,
            unknown3);
    }

    return encounters;
}


EncounterStore::EncounterStore(
    const EncounterFactory& ef,
    FileBuffer& fb,
    glm::uvec2 tile,
    unsigned tileIndex)
:
    mChapters{}
{
    mChapters.reserve(10);
    for (unsigned chapter = 1; chapter < 11; chapter++)
    {
        mChapters.emplace_back(
            LoadEncounters(
                ef,
                fb,
                Chapter{chapter},
                tile,
                tileIndex));
    }
}

const std::vector<Encounter>& EncounterStore::GetEncounters(Chapter chapter) const
{
    assert(chapter.mValue > 0 && chapter.mValue < 11);
    return mChapters[chapter.mValue - 1];
}

}
