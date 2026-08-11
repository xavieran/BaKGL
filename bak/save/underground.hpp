#pragma once

#include "bak/types.hpp"

#include <array>
#include <vector>

namespace BAK {

class FileBuffer;

constexpr auto sMaxUndergroundTiles = 40;
constexpr auto sUndergroundObjectFields = 38;
constexpr auto sBitsPerField = 8;

struct TileVisibility
{
    unsigned mZone{};
    unsigned mTileX{};
    unsigned mTileY{};
    std::array<bool, sUndergroundObjectFields * sBitsPerField> mVisibleObjects{};
};

std::vector<TileVisibility> LoadUnderground(FileBuffer&);
void Save(const std::vector<TileVisibility>&, FileBuffer&);

}
