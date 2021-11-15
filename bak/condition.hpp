#pragma once

#include "com/assert.hpp"
#include "com/saturatingNum.hpp"

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

constexpr std::uint16_t sConditionSkillEffect[7][6] = {
    {     1, 0xFFFF,      0,      0, 0, 0},  // Sick
    {     1, 0xFFFE,      0,      0, 0, 0},  // Plagued
    {     1, 0xFFFD,      0,      0, 0, 0},  // Poisoned
    {0xFFFE,      0, 0xFFF2, 0xFFC4, 0, 0},  // Drunk
    {0xFFFD,      1,      0,      0, 0, 0},  // Healing
    {     0, 0xFFFE,      0,      0, 0, 0},  // Starving
    {     0,      0,      0,      0, 0, 0}}; // Near Death

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
