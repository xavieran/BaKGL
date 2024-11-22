#pragma once

#include "bak/types.hpp"

#include "com/assert.hpp"

#include "graphics/glm.hpp"

#include <vector>

namespace BAK {

class FMapXY
{
public:
    static constexpr auto sFile = "FMAP_XY.DAT";

    FMapXY();
    glm::vec2 GetTileCoords(ZoneNumber zone, glm::uvec2 tile);

private:
    std::vector<std::vector<glm::uvec2>> mTiles;
    std::vector<std::vector<glm::vec2>> mTileCoords;
};

struct Town
{
    Town(
        std::string name,
        std::uint16_t type,
        glm::vec2 coord);

    std::string mName;
    std::uint16_t mType;
    glm::vec2 mCoord;
};

class FMapTowns
{
public:
    static constexpr auto sFile = "FMAP_TWN.DAT";

    FMapTowns();

    const std::vector<Town>& GetTowns() const;

private:
    std::vector<Town> mTowns;
};

}
