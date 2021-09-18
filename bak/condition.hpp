#pragma once

#include <array>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace BAK {

enum class Condition
{
    Sick      = 0,
    Plagued   = 1,
    Poisoned  = 2,
    Drunk     = 3,
    Healing   = 4,
    Starving  = 5,
    NearDeath = 6
};

std::string_view ToString(Condition);

class Conditions
{
public:
    static constexpr auto sNumConditions = 7;
    std::array<std::uint8_t, sNumConditions> mConditions;
};

std::ostream& operator<<(std::ostream&, const Conditions&);

}
