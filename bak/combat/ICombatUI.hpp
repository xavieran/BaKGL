#pragma once

#include "bak/combat/types.hpp"
#include "bak/types.hpp"

namespace BAK {

class ICombatUI
{
public:
    virtual void SetSelectedCharacter(CharIndex) = 0;
    virtual void DisplayMeleeInfo(MeleeInfo) = 0;
    virtual void ResetDisplay() = 0;

    virtual ~ICombatUI() = default;
};

}
