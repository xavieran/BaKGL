#include "game/combat/gridAlgorithms.hpp"

namespace Game {

unsigned ChebyshevDistance(glm::uvec2 src, glm::uvec2 dest)
{
    unsigned distance = 0;

    int sx = src.x;
    int sy = src.y;
    int dx = dest.x;
    int dy = dest.y;

    int distX = std::abs(sx - dx);
    int distY = std::abs(sy - dy);

    distance = std::max(distX, distY);
    return distance;
}

}
