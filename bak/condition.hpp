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

using ConditionValue = SaturatingNum<std::uint8_t, 0, 100>;

class Conditions
{
public:
    static constexpr auto sNumConditions = 7;
    std::array<ConditionValue, sNumConditions> mConditions;

    bool NoConditions() const
    {
        for (const auto cond : mConditions)
            if (cond != 0) return false;
        return true;
    }

    const ConditionValue& GetCondition(BAK::Condition cond) const
    {
        const auto i = static_cast<unsigned>(cond);
        ASSERT(i < sNumConditions);
        return mConditions[i];
    }

    void IncreaseCondition(BAK::Condition cond, signed value)
    {
        const auto i = static_cast<unsigned>(cond);
        ASSERT(i < sNumConditions);
        mConditions[i] += value;
    }
};

std::ostream& operator<<(std::ostream&, const Conditions&);

}
