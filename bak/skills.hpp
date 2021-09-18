#pragma once

#include <array>
#include <ostream>
#include <string_view>

namespace BAK {

enum class SkillType
{
    Health = 0,
    Stamina,
    Speed,
    Strength,
    Defense,
    Crossbow,
    Melee,
    Casting,
    Assessment,
    Armorcraft,
    Weaponcraft,
    Barding,
    Haggling,
    Lockpick,
    Scouting,
    Stealth,
    // FIXME: Need a better way to handle this this is the "GainHealth" GainSkill dialog action
    GainHealth
};

std::string_view ToString(SkillType);

struct Skill
{
    std::uint8_t mMax;
    std::uint8_t mCurrent;
    std::uint8_t mLimit;
    std::uint8_t mExperience;
    std::int8_t mModifier;
    bool mUnseenImprovement;
};

std::ostream& operator<<(std::ostream&, const Skill&);

struct Skills
{
    static constexpr auto sSkills = 16;
    using SkillArray = std::array<Skill, sSkills>;
    SkillArray mSkills;
};

std::ostream& operator<<(std::ostream&, const Skills&);

}
