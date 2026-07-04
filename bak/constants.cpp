#include "bak/constants.hpp"

namespace BAK {

std::string_view ToString(Terrain t)
{
    using enum Terrain;
    switch (t)
    {
        case Ground:    return "Ground";
        case Bank:      return "Bank";
        case Waterfall: return "Waterfall";
        case Path:      return "Path";
        case Dirt:      return "Dirt";
        case River:     return "River";
        case Sand:      return "Sand";
        case Road:      return "Road";
    }
    return "UNKNOWN";
}

}
