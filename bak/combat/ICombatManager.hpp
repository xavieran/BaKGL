#pragma once

#include "bak/types.hpp"

namespace BAK {

class ICombatManager
{
public:
    virtual void SetCastingSpell(SpellIndex) = 0;
    virtual void SetUsingCrossbow() = 0;

    virtual ~ICombatManager() = default;
};

}
