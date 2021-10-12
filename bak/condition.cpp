#include "bak/condition.hpp"

namespace BAK {

std::string_view ToString(Condition s)
{
    switch (s)
    {
    case Condition::Sick: return "Sick";
    case Condition::Plagued: return "Plagued";
    case Condition::Poisoned: return "Poisoned";
    case Condition::Drunk: return "Drunk";
    case Condition::Healing: return "Healing";
    case Condition::Starving: return "Starving";
    case Condition::NearDeath: return "NearDeath";
    default: return "UnknownCondition";
    }
}

std::ostream& operator<<(std::ostream& os, const Conditions& s)
{
    os << "Conditions: { ";
    char sep = ' ';
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        os << sep << ToString(static_cast<Condition>(i)) 
            << ": " << s.mConditions[i] << "%";
        sep = ',';
    }
    os << "}";
    return os;
}

}
