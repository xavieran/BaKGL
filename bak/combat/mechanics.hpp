#pragma once

#include <cstdint>

namespace BAK::Combat {

enum class CombatantState : std::uint8_t
{
    Alive = 0x1,
    Dead  = 0x2,
    Fled  = 0x10
};

}
