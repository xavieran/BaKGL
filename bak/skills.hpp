#pragma once

#include "com/assert.hpp"

#include "bak/condition.hpp"

#include <array>
#include <numeric>
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

static constexpr auto sEffectiveSkillMin = std::array<std::uint16_t, 16>{
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

constexpr std::uint16_t sSkillAbsMax = 0xfa;

constexpr std::uint8_t sSkillHealthEffect[16] = {0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2};
constexpr std::uint8_t sSkillExperienceVar1[16] = {3, 3, 1, 1, 2, 3, 1, 3, 8, 5, 5, 0x20, 2, 3, 8, 1};
constexpr std::uint8_t sSkillExperienceVar2[16] = {0x33, 0x33, 8, 8, 8, 0x33, 8, 0x33, 0, 0, 0, 0x80, 0x20, 0x33, 0, 0x40};

constexpr std::uint8_t sSkillMin[16] = {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
constexpr std::uint8_t sSkillMax[16] = {0xFA, 0xFA, 0xFA, 0xFA, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64};

constexpr std::uint8_t sTotalSelectedSkillPool = 0x1a;


std::string_view ToString(SkillType);

struct Skill
{
    std::uint8_t mMax;
    std::uint8_t mTrueSkill;
    std::uint8_t mCurrent;
    std::uint8_t mExperience;
    std::int8_t mModifier;
    bool mSelected;
    bool mUnseenImprovement;
};

std::ostream& operator<<(std::ostream&, const Skill&);

void DoImproveSkill(
    SkillType skillType,
    Skill& skill,
    unsigned skillChangeType,
    unsigned multiplier,
    unsigned selectedSkillPool);

struct Skills
{
    static constexpr auto sSkills = 16;
    using SkillArray = std::array<Skill, sSkills>;
    SkillArray mSkills;
    unsigned mSelectedSkillPool;

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
        mSelectedSkillPool = CalculateSelectedSkillPool();
    }

    void ClearUnseenImprovements()
    {
        for (auto& skill : mSkills)
            skill.mUnseenImprovement = false;
    }

    std::uint8_t CalculateSelectedSkillPool() const
    {
        const unsigned skillsSelected = std::accumulate(
            mSkills.begin(), mSkills.end(),
            0,
            [](const auto sum, const auto& elem){
                return sum + static_cast<unsigned>(elem.mSelected);
            });

        return skillsSelected > 0 
            ? sTotalSelectedSkillPool / skillsSelected
            : 0;
    }

    void ImproveSkill(
            BAK::SkillType skill, 
            unsigned skillChangeType,
            unsigned multiplier)
    {
        // not quite right...
        if (skill == SkillType::GainHealth)
        {
            skill = SkillType::Health;
            auto& s = GetSkill(skill);
            s.mMax += multiplier; 
            s.mTrueSkill += multiplier;
            s.mCurrent += multiplier;
            s.mUnseenImprovement = true;
        }
        else
        {
            DoImproveSkill(
                skill,
                GetSkill(skill),
                skillChangeType,
                multiplier,
                mSelectedSkillPool);
        }
    }
};

std::ostream& operator<<(std::ostream&, const Skills&);

unsigned CalculateEffectiveSkillValue(
    SkillType,
    Skills&,
    const Conditions&);

}
