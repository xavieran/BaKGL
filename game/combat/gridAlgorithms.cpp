#include "game/combat/gridAlgorithms.hpp"

#include "bak/coordinates.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include <algorithm>
#include <array>
#include <queue>

namespace Game::Combat {

bool IsAdjacent(GridPos src, GridPos dest)
{
    if (ChebyshevDistance(src, dest) != 1)
    {
        return false;
    }

    return (src.x == dest.x) || (src.y == dest.y);
}

unsigned ChebyshevDistance(GridPos src, GridPos dest)
{
    return std::max(
        std::abs(src.x - dest.x),
        std::abs(src.y - dest.y));
}

// We want to prioritise the direction the combatant is already
// facing when path finding
std::array<BAK::Direction, 8> GetNeighborOrder(BAK::Direction straight)
{
    auto s = static_cast<unsigned>(straight);
    return {{
        straight,
        static_cast<BAK::Direction>((s + 1) % 8),
        static_cast<BAK::Direction>((s + 7) % 8),
        static_cast<BAK::Direction>((s + 2) % 8),
        static_cast<BAK::Direction>((s + 6) % 8),
        static_cast<BAK::Direction>((s + 3) % 8),
        static_cast<BAK::Direction>((s + 5) % 8),
        static_cast<BAK::Direction>((s + 4) % 8),
    }};
}

std::vector<GridPos> FinalisePath(
    GridPos src,
    GridPos dest,
    const std::vector<GridPos>& parent,
    const Grid& grid)
{
    std::vector<GridPos> path{};
    auto back = dest;
    while (back != src)
    {
        path.push_back(back);
        back = parent[grid.GetIndex(back)];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<GridPos> CalculatePath(
    GridPos src,
    GridPos dest,
    const Grid& grid)
{
    if (src == dest)
        return {};

    const auto cols = grid.GetCols();
    const auto rows = grid.GetRows();

    std::vector<bool> visited(cols * rows, false);
    std::vector<GridPos> parent{cols * rows};
    std::queue<GridPos> frontier{};

    visited[grid.GetIndex(src)] = true;
    frontier.push(src);

    auto directions = GetNeighborOrder(
            BAK::GetDirectionBetween(
                BAK::GamePosition(src), BAK::GamePosition(dest)));
    while (!frontier.empty())
    {
        auto current = frontier.front();
        frontier.pop();

        for (auto dir : directions)
        {
            auto candidate = current + BAK::ToDelta(dir);

            if (!grid.WithinBounds(candidate))
                continue;

            if (candidate == dest)
            {
                parent[grid.GetIndex(dest)] = current;
                return FinalisePath(src, dest, parent, grid);
            }

            auto candidateIdx = grid.GetIndex(candidate);
            if (!visited[candidateIdx] && grid.CanMoveTo(candidate))
            {
                visited[candidateIdx] = true;
                parent[candidateIdx] = current;
                frontier.push(candidate);
            }
        }
    }

    return {};
}

std::optional<GridPos> SelectBestAttackPosition(GridPos src, GridPos target, const Grid& grid)
{
    auto dirFromTargetToMe = BAK::GetDirectionBetween(
            BAK::GamePosition(target),
            BAK::GamePosition(src));
    auto moveTo = target + BAK::ToDelta(dirFromTargetToMe);

    if (moveTo == src && BAK::IsCardinal(dirFromTargetToMe))
    {
        return src;
    }

    std::vector<GridPos> candidates{
        target + BAK::ToDelta(BAK::Direction::North),
        target + BAK::ToDelta(BAK::Direction::South),
        target + BAK::ToDelta(BAK::Direction::East),
        target + BAK::ToDelta(BAK::Direction::West),
    };

    std::sort(candidates.begin(), candidates.end(), [&](const auto& l, const auto& r)
    {
        auto pathL = CalculatePath(src, l, grid).size();
        auto pathR = CalculatePath(src, r, grid).size();
        return pathL < pathR;
    });

    for (const auto& candidate : candidates)
    {
        if (grid.CanMoveTo(candidate))
        {
            return candidate;
        }
    }

    return std::nullopt;
}

}
