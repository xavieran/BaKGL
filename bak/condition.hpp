#pragma once

#include "com/saturatingNum.hpp"

#include <array>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace BAK {

class Skills;

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

// deterioration
// heal reduction
// ....
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

    bool NoConditions() const;

    const ConditionValue& GetCondition(BAK::Condition cond) const;

    void IncreaseCondition(BAK::Condition cond, signed value);

    void AdjustCondition(Skills&, BAK::Condition, signed amount);
    void SetCondition(BAK::Condition cond, std::uint8_t amount);
};

std::ostream& operator<<(std::ostream&, const Conditions&);

}
