#pragma once

#include "bak/types.hpp"

namespace BAK {

class ICombatUI
{
public:
    virtual void SetSelectedCharacter(CharIndex) = 0;

    virtual ~ICombatUI() = default;
};

}
