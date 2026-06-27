#pragma once

#include "game/combat/types.hpp"
#include "game/combat/grid.hpp"

#include <glm/glm.hpp>

namespace Game::Combat {

bool IsAdjacent(GridPos src, GridPos dest);
unsigned ChebyshevDistance(GridPos src, GridPos dest);
std::vector<GridPos> CalculatePath(GridPos src, GridPos dest, const Grid&);
std::optional<GridPos> SelectBestAttackPosition(GridPos src, GridPos target, const Grid&);

}
