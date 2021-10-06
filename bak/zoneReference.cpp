#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include "xbak/FileBuffer.h"

namespace BAK {

std::vector<glm::vec<2, unsigned>> LoadZoneRef(const std::string& path)
{
    auto fb = FileBufferFactory::CreateFileBuffer(path);
    const auto logger = Logging::LogState::GetLogger("LoadZoneRef");

    const auto numberTiles = fb.GetUint8();
    logger.Debug() << "Number of tiles: " << numberTiles << "\n";

    std::vector<glm::vec<2, unsigned>> tiles{};
    tiles.reserve(numberTiles);

    for (unsigned i = 0; i < numberTiles; i++)
    {
        const auto& tile = tiles.emplace_back(
            glm::vec<2, unsigned>{
                fb.GetUint8(),
                fb.GetUint8()});
        logger.Debug() << std::hex << tile << " " << i << std::dec << "\n";
    }

    return tiles;
}

}
