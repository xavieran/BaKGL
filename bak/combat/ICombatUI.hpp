#pragma once

#include "bak/types.hpp"

namespace BAK {

class ICombatUI
{
public:
    virtual void SetSelectedCharacter(ActiveCharIndex) = 0;

    virtual ~ICombatUI() = default;
};

}
