#include "bak/skills.hpp"

#include "com/logger.hpp"

namespace BAK {

std::string_view ToString(SkillType s)
{
    switch (s)
    {
    case SkillType::Health: return "Health";
    case SkillType::Stamina: return "Stamina";
    case SkillType::Speed: return "Speed";
    case SkillType::Strength: return "Strength";
    case SkillType::Defense: return "Defense";
    case SkillType::Crossbow: return "Crossbow";
    case SkillType::Melee: return "Melee";
    case SkillType::Casting: return "Casting";
    case SkillType::Assessment: return "Assessment";
    case SkillType::Armorcraft: return "Armorcraft";
    case SkillType::Weaponcraft: return "Weaponcraft";
    case SkillType::Barding: return "Barding";
    case SkillType::Haggling: return "Haggling";
    case SkillType::Lockpick: return "Lockpick";
    case SkillType::Scouting: return "Scouting";
    case SkillType::Stealth: return "Stealth";
    case SkillType::GainHealth: return "GainHealth";
    default: return "UnknownSkillType";
    }
}

std::ostream& operator<<(std::ostream& os, const Skill& s)
{
    os << "{ Max: " << +s.mMax << " TrueSkill: " << +s.mTrueSkill 
        << " Current: " << +s.mCurrent << " Experience: " << +s.mExperience
        << " Modifier: " << +s.mModifier << "[";
    if (s.mSelected) os << "*";
    else os << " ";
    os << "] [";
    if (s.mUnseenImprovement) os << "*";
    else os << " ";
    os << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Skills& s)
{
    for (unsigned i = 0; i < Skills::sSkills; i++)
    {
        os << ToString(static_cast<SkillType>(i)) << " " << s.mSkills[i] << "\n";
    }
        os << "SelectedSkillPool: " << s.mSelectedSkillPool << "\n";
    return os;
}

unsigned CalculateEffectiveSkillValue(
    SkillType skillType,
    Skills& skills,
    const Conditions& conditions)
{
    const auto skillIndex = static_cast<unsigned>(skillType);
    auto& skill = skills.GetSkill(skillType);

    int skillCurrent = skill.mTrueSkill;

    if (skill.mModifier != 0)
        skillCurrent += skill.mModifier;

    if (skillCurrent < 0)
        skillCurrent = 0;

    // This checks the effect of potions 
    // FIXME: Will get to this when I do combat since potions
    // only affect combat skills
    for (unsigned i = 0 ; i < 8; i++)
        ;

    for (unsigned i = 0 ; i < 7; i++)
    {
        const auto condition = static_cast<Condition>(i);
        const auto conditionAmount = conditions.GetCondition(condition).Get();
        const std::uint16_t skillBitOffset = 1 << skillIndex;
        if (conditionAmount != 0)
        {
            if (sConditionSkillEffect[i][2] & skillBitOffset)
            {
                auto effect = 0xffff - sConditionSkillEffect[i][3];
                effect *= conditionAmount;
                effect /= 100;
                effect = 100 - effect;
                
                auto effectedSkill = (effect * skillCurrent) / 100;
                skillCurrent = effectedSkill;
            }

            // if drunk[4] * skillBitOffset) ... do the same as above with drunk[5]
        }
    }

    const auto skillHealthEffect = sSkillHealthEffect[skillIndex];
    if (skillHealthEffect != 0)
    {
        const auto& health = skills.GetSkill(SkillType::Health);
        auto trueHealth = health.mTrueSkill;
        auto maxHealth  = health.mMax;
        if (!(skillHealthEffect <= 1))
        {
            trueHealth = (((skillHealthEffect - 1) 
                * trueHealth) 
                    + maxHealth) 
                / skillHealthEffect;
        }

        skillCurrent = (((skillCurrent 
            * trueHealth)
                + maxHealth) - 1)
            / maxHealth;
    }

    if (skillCurrent > sSkillCaps[skillIndex])
        skillCurrent = sSkillCaps[skillIndex];

    if (skillCurrent > sSkillAbsMax)
        skillCurrent = sSkillAbsMax;

    if (skillCurrent < sEffectiveSkillMin[skillIndex])
        skillCurrent = sEffectiveSkillMin[skillIndex];

    skill.mCurrent = skillCurrent;

    return skillCurrent;
}

void DoImproveSkill(
    SkillType skillType,
    Skill& skill,
    SkillChange skillChangeType,
    unsigned multiplier,
    unsigned selectedSkillPool)
{
    if (skill.mMax == 0) return;

    const auto skillIndex = static_cast<unsigned>(skillType);

    const auto initialSkillValue = skill.mTrueSkill;

    int experienceChange = 0;

    switch (skillChangeType)
    {
    case SkillChange::ExercisedSkill:
    {
        // FIXME: double check this... obviously these
        // numbers can go negative for many of the vals
        // in the array
        const auto diff = sSkillExperienceVar1[skillIndex]
            - sSkillExperienceVar2[skillIndex];
        experienceChange = ((diff * skill.mTrueSkill) / 100)
            + sSkillExperienceVar2[skillIndex];

        if (multiplier != 0)
            experienceChange *= multiplier;
    } break;
    case SkillChange::DifferenceOfSkill:
        experienceChange = (100 - skill.mTrueSkill) * multiplier;
        break;
    case SkillChange::FractionOfSkill:
        experienceChange = (skill.mTrueSkill * multiplier) / 100;
        break;
    case SkillChange::Direct:
        experienceChange = multiplier;
        break;
    }

    // di + 0x58
    bool realChar = true;
    if (realChar)
    {
        if (skill.mSelected)
        {
            const auto bonus = (experienceChange * selectedSkillPool) 
                / (sTotalSelectedSkillPool * 2);
            experienceChange += bonus;
        }
    }

    experienceChange += skill.mExperience;
    auto levels = experienceChange / 256;
    auto leftoverExperience = experienceChange % 256;
    skill.mExperience = leftoverExperience;

    if (levels < 0)
    {
        // fill this out...
        // offset 0x573
    }

    skill.mTrueSkill = levels + skill.mTrueSkill;

    if (skill.mTrueSkill < sSkillMin[skillIndex])
        skill.mTrueSkill = sSkillMin[skillIndex];

    if (skill.mTrueSkill > sSkillMax[skillIndex])
        skill.mTrueSkill = sSkillMax[skillIndex];

    if (skill.mTrueSkill > skill.mMax)
        skill.mMax = skill.mTrueSkill;

    if (initialSkillValue != skill.mTrueSkill)
        skill.mUnseenImprovement = true;

    Logging::LogDebug(__FUNCTION__) << "SkillImproved: " 
        << ToString(skillType) << " " << skill << "\n";
}

Skills::Skills(const SkillArray& skills, unsigned pool)
:
    mSkills{skills},
    mSelectedSkillPool{pool}
{}
const Skill& Skills::GetSkill(BAK::SkillType skill) const
{
    const auto i = static_cast<unsigned>(skill);
    ASSERT(i < sSkills);
    return mSkills[i];
}

Skill& Skills::GetSkill(BAK::SkillType skill)
{
    const auto i = static_cast<unsigned>(skill);
    ASSERT(i < sSkills);
    return mSkills[i];
}

void Skills::SetSkill(BAK::SkillType skillType, const Skill& skill)
{
    const auto i = static_cast<unsigned>(skillType);
    mSkills[i] = skill;
}

void Skills::SetSelectedSkillPool(unsigned pool)
{
    mSelectedSkillPool = pool;
}

void Skills::ToggleSkill(BAK::SkillType skillType)
{
    auto& skill = GetSkill(skillType);
    skill.mSelected = !skill.mSelected;
    mSelectedSkillPool = CalculateSelectedSkillPool();
}

void Skills::ClearUnseenImprovements()
{
    for (auto& skill : mSkills)
        skill.mUnseenImprovement = false;
}

std::uint8_t Skills::CalculateSelectedSkillPool() const
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

void Skills::ImproveSkill(
    SkillType skill, 
    SkillChange skillChangeType,
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

}
