#pragma once

#include "com/assert.hpp"
#include "com/strongType.hpp"

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
    TotalHealth = 0x10
};

enum class SkillChange
{
    Direct = 0,
    FractionOfSkill = 1,
    DifferenceOfSkill = 2,
    ExercisedSkill = 3,

    HealMultiplier_80 = 80,
    HealMultiplier_100 = 100
};

enum class SkillRead
{
    Current = 0,
    MaxSkill = 1,
    TrueSkill = 3,
    NoHealthEffect = 4
};

static constexpr auto sEffectiveSkillMin = std::array<std::uint16_t, 16>{
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static constexpr auto sSkillCaps = std::array<std::uint16_t, 16>{
    0x1f4, 0x1f4, 0x1f4, 0x1f4,
    0x0c8, 0x0c8, 0x0c8, 0x0c8,
    0x0c8, 0x0c8, 0x0c8, 0x0c8,
    0x0c8, 0x064, 0x0c8, 0x0c8};

constexpr std::uint16_t sSkillAbsMax = 0xfa;

constexpr std::uint8_t sSkillHealthEffect[16] = {
    0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2};
constexpr std::uint8_t sSkillExperienceVar1[16] = {
       3,    3, 1, 1, 2,    3, 1,    3, 8, 5, 5, 0x20,    2,    3, 8,    1};
constexpr std::uint8_t sSkillExperienceVar2[16] = {
    0x33, 0x33, 8, 8, 8, 0x33, 8, 0x33, 0, 0, 0, 0x80, 0x20, 0x33, 0, 0x40};

constexpr std::uint8_t sSkillMin[16] = {
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
constexpr std::uint8_t sSkillMax[16] = {
    0xFA, 0xFA, 0xFA, 0xFA,
    0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64};

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

class Skills;

unsigned CalculateEffectiveSkillValue(
    SkillType,
    Skills&,
    const Conditions&,
    SkillRead);

void DoImproveSkill(
    SkillType skillType,
    Skill& skill,
    SkillChange skillChangeType,
    unsigned multiplier,
    unsigned selectedSkillPool);

signed DoAdjustHealth(
    Skills& skills,
    Conditions& conditions,
    signed healthChangePercent,
    signed multiplier);

class Skills
{
public:
    static constexpr auto sSkills = 16;
    using SkillArray = std::array<Skill, sSkills>;

    Skills(const SkillArray&, unsigned);

    Skills() = default;
    Skills(const Skills&) = default;
    Skills& operator=(const Skills&) = default;
    Skills(Skills&&) = default;
    Skills& operator=(Skills&&) = default;

    const Skill& GetSkill(SkillType skill) const;
    Skill& GetSkill(SkillType skill);
    void SetSkill(BAK::SkillType skillType, const Skill& skill);
    void SetSelectedSkillPool(unsigned);
    
    void ToggleSkill(BAK::SkillType skillType);

    void ClearUnseenImprovements();

    std::uint8_t CalculateSelectedSkillPool() const;

    void ImproveSkill(
        SkillType skill, 
        SkillChange skillChangeType,
        int multiplier);

    friend std::ostream& operator<<(std::ostream&, const Skills&);
private:
    SkillArray mSkills;
    unsigned mSelectedSkillPool;
};

std::ostream& operator<<(std::ostream&, const Skills&);

}
