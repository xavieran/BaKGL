#include "game/combat/actionQueue.hpp"

#include <vector>

namespace Game::Combat {

std::ostream& operator<<(std::ostream& os, const ActionQueue& aq)
{
    os << "ActionQueue {";
    auto copy = aq.mActions;
    bool first = true;
    while (!copy.empty())
    {
        if (!first)
            os << ", ";
        os << copy.front();
        copy.pop();
        first = false;
    }
    os << "}";
    return os;
}

}
