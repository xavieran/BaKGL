#pragma once

#include "com/assert.hpp"
#include "com/saturatingNum.hpp"

#include <array>
#include <cstdint>
#include <limits>
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

    bool NoConditions() const
    {
        for (const auto cond : mConditions)
            if (cond != 0) return false;
        return true;
    }

    unsigned GetCondition(BAK::Condition cond) const
    {
        const auto i = static_cast<unsigned>(cond);
        ASSERT(i < sNumConditions);
        return mConditions[i];
    }

    void IncreaseCondition(BAK::Condition cond, signed value)
    {
        const auto i = static_cast<unsigned>(cond);
        ASSERT(i < sNumConditions);
        const int newVal = mConditions[i] + value;
        if (newVal > std::numeric_limits<std::uint8_t>::max())
        {
            mConditions[i] = 100;
        }
        else if (newVal < 0)
        {
            mConditions[i] = 0;
        }
        else
        {
            mConditions[i] = newVal;
        }
    }
};

std::ostream& operator<<(std::ostream&, const Conditions&);

}
