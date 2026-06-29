#include "game/combat/types.hpp"

namespace Game::Combat {

std::ostream& operator<<(std::ostream& os, const Move& move)
{
    return os << "Move {" << move.mTarget << "}";
}

std::ostream& operator<<(std::ostream& os, const Attack& attack)
{
    return os << "Attack {" << attack.mTarget << ", " << attack.mType << "}";
}

std::ostream& operator<<(std::ostream& os, const AnimateDeath& animateDeath)
{
    return os << "AnimateDeath {" << animateDeath.mTarget << "}";
}

std::ostream& operator<<(std::ostream& os, const CombatAction& action)
{
    std::visit([&os](const auto& a){ os << a; }, action);
    return os;
}

}
