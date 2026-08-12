#pragma once

#include "bak/types.hpp"

#include <glm/glm.hpp>

#include <array>
#include <optional>
#include <vector>

namespace BAK {

class FileBuffer;

constexpr auto sMaxUndergroundTiles = 40;
constexpr auto sUndergroundObjectFields = 38;
constexpr auto sBitsPerField = 8;
constexpr auto sNoEntrySentinel = 0xff;

struct TileLocation
{
    unsigned mZone{};
    glm::uvec2 mTile{};
    bool operator==(const TileLocation&) const = default;
};

struct TileVisibility
{
    std::optional<TileLocation> mTileLocation{};
    std::array<bool, sUndergroundObjectFields * sBitsPerField> mVisibleObjects{};
};

std::vector<TileVisibility> LoadTileVisibility(FileBuffer&);
void Save(const std::vector<TileVisibility>&, FileBuffer&);

TileVisibility& EnsureTileVisibility(
    std::vector<TileVisibility>& entries,
    const TileLocation& location);

const TileVisibility* LookupTileVisibility(
    const std::vector<TileVisibility>& entries,
    const TileLocation& location);

}
