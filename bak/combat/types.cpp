#include "bak/combat/types.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, AttackType type)
{
    switch (type)
    {
    case AttackType::Slash: return os << "Slash";
    case AttackType::Thrust: return os << "Thrust";
    }
    return os;
}

}
