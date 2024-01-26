#include "gtest/gtest.h"

#include "bak/camp.hpp"
#include "bak/condition.hpp"
#include "bak/skills.hpp"

#include "com/logger.hpp"

namespace BAK {

struct GameTimeTestFixture : public ::testing::Test
{
    GameTimeTestFixture()
    :
        mSkills{{
            Skill{55, 55, 55, 0, 0, false, false},
            Skill{45, 45, 45, 0, 0, false, false},
            Skill{ 4,  4,  4, 0, 0, false, false},
            Skill{17, 17, 17, 0, 0, false, false},
            Skill{80, 80, 80, 0, 0, false, false}, // Def
            Skill{80, 80, 80, 0, 0, false, false}, // Cro
            Skill{80, 80, 80, 0, 0, false, false}, // Mel
            Skill{ 0,  0,  0, 0, 0, false, false}, // Cast
            Skill{80, 80, 80, 0, 0, false, false}, // Assess
            Skill{80, 80, 80, 0, 0, false, false}, // Arm
            Skill{80, 80, 80, 0, 0, false, false}, // Wea
            Skill{80, 80, 80, 0, 0, false, false}, // Bard
            Skill{80, 80, 80, 0, 0, false, false}, // Haggle
            Skill{80, 80, 80, 0, 0, false, false}, // Lockpick
            Skill{80, 80, 80, 0, 0, false, false}, // Scout
            Skill{80, 80, 80, 0, 0, false, false}}, // Stealth
            0
        }
        , mConditions{
            ConditionValue{0},
            ConditionValue{0},
            ConditionValue{0},
            ConditionValue{0},
            ConditionValue{0},
            ConditionValue{0},
            ConditionValue{0}}
    {
    }

protected:
    void SetUp() override
    {
        //Logging::LogState::SetLevel(Logging::LogLevel::Fatal);
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    }

    Skills mSkills;
    Conditions mConditions;
};


TEST_F(GameTimeTestFixture, EffectOfConditionsWithTime_Sick)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 10;
    mConditions.SetCondition(BAK::Condition::Sick, 10);

    EffectOfConditionsWithTime(mSkills, mConditions, 0);

    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::Sick), 11);
    EXPECT_EQ(mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill, 9);
}

TEST_F(GameTimeTestFixture, EffectOfConditionsWithTime_Plagued)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 10;
    mConditions.SetCondition(BAK::Condition::Plagued, 11);

    EffectOfConditionsWithTime(mSkills, mConditions, 0);

    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::Plagued), 12);
    EXPECT_EQ(mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill, 8);
}

TEST_F(GameTimeTestFixture, EffectOfConditionsWithTime_Poisoned)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 10;
    mConditions.SetCondition(BAK::Condition::Poisoned, 11);

    EffectOfConditionsWithTime(mSkills, mConditions, 0);

    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::Poisoned), 12);
    EXPECT_EQ(mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill, 7);
}

TEST_F(GameTimeTestFixture, EffectOfConditionsWithTime_Healing)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 10;
    mConditions.SetCondition(BAK::Condition::Healing, 10);

    EffectOfConditionsWithTime(mSkills, mConditions, 0);

    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::Healing), 7);
    EXPECT_EQ(mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill, 11);
}

TEST_F(GameTimeTestFixture, EffectOfConditionsWithTime_Multiple)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 10;
    mConditions.SetCondition(BAK::Condition::Sick, 10);
    mConditions.SetCondition(BAK::Condition::Plagued, 10);
    mConditions.SetCondition(BAK::Condition::Poisoned, 10);
    mConditions.SetCondition(BAK::Condition::Healing, 10);

    EffectOfConditionsWithTime(mSkills, mConditions, 0);

    mConditions.SetCondition(BAK::Condition::Sick, 9);
    mConditions.SetCondition(BAK::Condition::Plagued, 8);
    mConditions.SetCondition(BAK::Condition::Poisoned, 7);
    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::Healing), 7);
    EXPECT_EQ(mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill, 5);
}

TEST_F(GameTimeTestFixture, EffectOfConditionsWithTime_MultipleInInn)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 10;
    mConditions.SetCondition(BAK::Condition::Sick, 10);
    mConditions.SetCondition(BAK::Condition::Plagued, 10);
    mConditions.SetCondition(BAK::Condition::Poisoned, 10);
    mConditions.SetCondition(BAK::Condition::Healing, 10);

    EffectOfConditionsWithTime(mSkills, mConditions, 100);

    mConditions.SetCondition(BAK::Condition::Sick, 6);
    mConditions.SetCondition(BAK::Condition::Plagued, 8);
    mConditions.SetCondition(BAK::Condition::Poisoned, 7);
    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::Healing), 7);
    EXPECT_EQ(mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill, 7);
}

TEST_F(GameTimeTestFixture, NearDeathHeal)
{
    // Normal progression
    mConditions.SetCondition(BAK::Condition::NearDeath, 100);
    ImproveNearDeath(mSkills, mConditions);
    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::NearDeath), 99);

    // With healing
    mConditions.SetCondition(BAK::Condition::NearDeath, 70);
    mConditions.SetCondition(BAK::Condition::Healing, 1);
    ImproveNearDeath(mSkills, mConditions);
    EXPECT_EQ(mConditions.GetCondition(BAK::Condition::NearDeath), 62);
}
}
