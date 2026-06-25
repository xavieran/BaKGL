#pragma once

#include "game/combat/types.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <queue>

namespace Game::Combat {

class ActionQueue
{
public:
    void Push(CombatAction action)
    {
        mActions.push(action);
    }

    CombatAction Pop()
    {
        assert(!mActions.empty());
        auto action = mActions.front();
        mActions.pop();
        return action;
    }

    bool HasAction() const
    {
        return !mActions.empty();
    }

    std::queue<CombatAction> mActions{};

    friend std::ostream& operator<<(std::ostream& os, const ActionQueue& aq);
};

std::ostream& operator<<(std::ostream& os, const ActionQueue& aq);

}
