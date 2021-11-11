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

static constexpr auto sSkillCaps = std::array<std::uint16_t, 16>{
    0x1f4,
    0x1f4,
    0x1f4,
    0x1f4,
    0x0c8,
    0x0c8,
    0x0c8,
    0x0c8,
    0x0c8,
    0x0c8,
    0x0c8,
    0x0c8,
    0x0c8,
    0x064,
    0x0c8,
    0x0c8};

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

    const Skill& GetSkill(BAK::SkillType skill) const
    {
        const auto i = static_cast<unsigned>(skill);
        ASSERT(i < sSkills);
        return mSkills[i];
    }

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

    void ClearUnseenImprovements()
    {
        for (auto& skill : mSkills)
            skill.mUnseenImprovement = false;
    }

    void ImproveSkill(BAK::SkillType skill, unsigned value)
    {
        // not quite right...
        if (skill == SkillType::GainHealth)
            skill = SkillType::Health;

        auto& s = GetSkill(skill);
        // FIXME: Check for overflow...
        // FIXME: Account for whether this skill is selected or not...
        s.mMax += value; 
        s.mCurrent += value;
        s.mLimit += value;
        s.mUnseenImprovement = true;
    }
};

std::ostream& operator<<(std::ostream&, const Skills&);

}
