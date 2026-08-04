#pragma once

#include "bak/types.hpp"

#include <glm/glm.hpp>

namespace BAK {

class ICombatManager
{
public:
    virtual void SetCastingSpell(SpellIndex) = 0;
    virtual void SetUsingCrossbow() = 0;
    virtual void DoDefend() = 0;
    virtual void DoRest() = 0;
    virtual void DoAutobattle() = 0;
    virtual void DoFlee() = 0;

    virtual void CompleteMove(glm::ivec2) = 0;
    virtual void CompleteAttack(glm::ivec2) = 0;

    virtual ~ICombatManager() = default;
};

}
