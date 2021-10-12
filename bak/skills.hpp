#pragma once

#include "com/assert.hpp"

#include <array>
#include <ostream>
#include <string_view>

namespace BAK {

enum class SkillType
{
    Health      = 0,
    Stamina     = 1,
    Speed       = 2,
    Strength    = 3,
    Defense     = 4,
    Crossbow    = 5,
    Melee       = 6,
    Casting     = 7,
    Assessment  = 8,
    Armorcraft  = 9,
    Weaponcraft = 0xa,
    Barding     = 0xb,
    Haggling    = 0xc,
    Lockpick    = 0xd,
    Scouting    = 0xe,
    Stealth     = 0xf,
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
    bool mSelected;
    bool mUnseenImprovement;
};

std::ostream& operator<<(std::ostream&, const Skill&);

struct Skills
{
    static constexpr auto sSkills = 16;
    using SkillArray = std::array<Skill, sSkills>;
    SkillArray mSkills;

    Skill& GetSkill(BAK::SkillType skill)
    {
        const auto i = static_cast<unsigned>(skill);
        ASSERT(i < sSkills);
        return mSkills[i];
    }
    
    void ToggleSkill(BAK::SkillType skillType)
    {
        auto& skill = GetSkill(skillType);
        skill.mSelected = !skill.mSelected;
    }
};

std::ostream& operator<<(std::ostream&, const Skills&);

}
