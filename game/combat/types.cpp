#include "game/combat/types.hpp"

namespace Game::Combat {

std::ostream& operator<<(std::ostream& os, Move move)
{
    return os << "Move {" << move.mTarget << "}";
}

std::ostream& operator<<(std::ostream& os, AttackType type)
{
    switch (type)
    {
    case AttackType::Slash: return os << "Slash";
    case AttackType::Thrust: return os << "Thrust";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Attack& attack)
{
    return os << "Attack {" << attack.mTarget << ", " << attack.mType << "}";
}

std::ostream& operator<<(std::ostream& os, const CombatAction& action)
{
    std::visit([&os](const auto& a){ os << a; }, action);
    return os;
}

}
