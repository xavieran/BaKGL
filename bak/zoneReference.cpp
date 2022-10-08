#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include "xbak/FileBuffer.h"

namespace BAK {

std::vector<glm::uvec2> LoadZoneRef(const std::string& path)
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(path);
    const auto logger = Logging::LogState::GetLogger("LoadZoneRef");

    const auto numberTiles = fb.GetUint8();
    logger.Spam() << "Number of tiles: " << numberTiles << "\n";

    std::vector<glm::uvec2> tiles{};
    tiles.reserve(numberTiles);

    for (unsigned i = 0; i < numberTiles; i++)
    {
        const auto& tile = tiles.emplace_back(
            fb.LoadVector<std::uint8_t, 2>());
        logger.Spam() << std::hex << tile << " " << i << std::dec << "\n";
    }

    return tiles;
}

}
