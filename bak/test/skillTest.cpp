#include "gtest/gtest.h"

#include "bak/condition.hpp"
#include "bak/skills.hpp"

#include "com/logger.hpp"

namespace BAK {

struct SkillTestFixture : public ::testing::Test
{
    SkillTestFixture()
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

TEST_F(SkillTestFixture, CalculateEffectiveSkillValueTest)
{
    auto skillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
    {
        skillValues.emplace_back(
            CalculateEffectiveSkillValue(
                static_cast<BAK::SkillType>(i),
                mSkills,
                mConditions,
                SkillRead::Current));
    }

    auto expectedSkillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
        expectedSkillValues.emplace_back(mSkills.GetSkill(static_cast<SkillType>(i)).mTrueSkill);

    EXPECT_EQ(skillValues, expectedSkillValues);
}

TEST_F(SkillTestFixture, ConditionNonDrunkTest)
{
    mConditions.IncreaseCondition(BAK::Condition::Sick, 50);
    mConditions.IncreaseCondition(BAK::Condition::Plagued, 50);
    mConditions.IncreaseCondition(BAK::Condition::Poisoned, 50);
    mConditions.IncreaseCondition(BAK::Condition::Starving, 50);
    mConditions.IncreaseCondition(BAK::Condition::NearDeath, 50);


    auto skillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
    {
        skillValues.emplace_back(
            CalculateEffectiveSkillValue(
                static_cast<BAK::SkillType>(i),
                mSkills,
                mConditions,
                SkillRead::Current));
    }

    // Sickness doesn't affect stats...
    auto expectedSkillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
        expectedSkillValues.emplace_back(mSkills.GetSkill(static_cast<SkillType>(i)).mTrueSkill);

    EXPECT_EQ(skillValues, expectedSkillValues);
}

TEST_F(SkillTestFixture, ConditionDrunkTest)
{
    mConditions.IncreaseCondition(BAK::Condition::Drunk, 50);

    auto skillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
    {
        skillValues.emplace_back(
            CalculateEffectiveSkillValue(
                static_cast<BAK::SkillType>(i),
                mSkills,
                mConditions,
                SkillRead::Current));
    }

    auto expectedSkillValues = std::vector<unsigned>{
        55, 31, 4, 17};
    for (unsigned i = 4; i < 16; i++)
        expectedSkillValues.emplace_back(56);
    expectedSkillValues[static_cast<unsigned>(BAK::SkillType::Casting)] = 0;

    EXPECT_EQ(skillValues, expectedSkillValues);
}

TEST_F(SkillTestFixture, ConditionHealingTest)
{
    mConditions.IncreaseCondition(BAK::Condition::Healing, 50);

    auto skillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
    {
        skillValues.emplace_back(
            CalculateEffectiveSkillValue(
                static_cast<BAK::SkillType>(i),
                mSkills,
                mConditions,
                SkillRead::Current));
    }

    auto expectedSkillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
        expectedSkillValues.emplace_back(mSkills.GetSkill(static_cast<SkillType>(i)).mTrueSkill);

    EXPECT_EQ(skillValues, expectedSkillValues);
}

TEST_F(SkillTestFixture, LowHealthTest)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mCurrent = 0;
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 0;

    mSkills.GetSkill(BAK::SkillType::Health).mCurrent = 32;
    mSkills.GetSkill(BAK::SkillType::Health).mTrueSkill = 32;

    auto skillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
    {
        skillValues.emplace_back(
            CalculateEffectiveSkillValue(
                static_cast<BAK::SkillType>(i),
                mSkills,
                mConditions,
                SkillRead::Current));
    }

    auto expectedSkillValues = std::vector<unsigned>{
        32, 0, 3, 10};
    for (unsigned i = 4; i < 9; i++)
        expectedSkillValues.emplace_back(47);
    for (unsigned i = 9; i < 16; i++)
        expectedSkillValues.emplace_back(63);
    expectedSkillValues[static_cast<unsigned>(BAK::SkillType::Casting)] = 0;

    EXPECT_EQ(skillValues, expectedSkillValues);
}

TEST_F(SkillTestFixture, LowHealthAndDrunkAndModifierTest)
{
    mSkills.GetSkill(BAK::SkillType::Stamina).mCurrent = 0;
    mSkills.GetSkill(BAK::SkillType::Stamina).mTrueSkill = 0;

    mSkills.GetSkill(BAK::SkillType::Health).mCurrent = 32;
    mSkills.GetSkill(BAK::SkillType::Health).mTrueSkill = 32;

    mConditions.IncreaseCondition(BAK::Condition::Drunk, 50);

    mSkills.GetSkill(BAK::SkillType::Lockpick).mModifier += 15;

    auto skillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
    {
        skillValues.emplace_back(
            CalculateEffectiveSkillValue(
                static_cast<BAK::SkillType>(i),
                mSkills,
                mConditions,
                SkillRead::Current));
    }

    auto expectedSkillValues = std::vector<unsigned>{
        32, 0, 3, 10};
    for (unsigned i = 4; i < 9; i++)
        expectedSkillValues.emplace_back(33);
    for (unsigned i = 9; i < 16; i++)
        expectedSkillValues.emplace_back(44);
    // actually 52 but, close enough for my liking...
    expectedSkillValues[static_cast<unsigned>(BAK::SkillType::Lockpick)] = 53;
    expectedSkillValues[static_cast<unsigned>(BAK::SkillType::Casting)] = 0;

    EXPECT_EQ(skillValues, expectedSkillValues);
}

TEST_F(SkillTestFixture, ImproveSkillTest)
{
    auto& skill = mSkills.GetSkill(BAK::SkillType::Lockpick);
    skill.mMax       = 24;
    skill.mCurrent   = 24;
    skill.mTrueSkill = 24;

    for (unsigned i = 0; i < 3; i++)
        mSkills.ImproveSkill(
            BAK::SkillType::Lockpick, 
            BAK::SkillChange::ExercisedSkill,
            2);
    EXPECT_EQ(skill.mTrueSkill, 24);
    EXPECT_EQ(skill.mExperience, 0xf0);
    EXPECT_EQ(skill.mUnseenImprovement, false);

    mSkills.ToggleSkill(BAK::SkillType::Lockpick);

    mSkills.ImproveSkill(
            BAK::SkillType::Lockpick, 
            BAK::SkillChange::ExercisedSkill,
            2);

    EXPECT_EQ(skill.mTrueSkill, 25);
    EXPECT_EQ(skill.mExperience, 104);
    EXPECT_EQ(skill.mUnseenImprovement, true);
}

TEST_F(SkillTestFixture, ImproveSkillFromDialogTest)
{
    auto& skill = mSkills.GetSkill(BAK::SkillType::Lockpick);
    skill.mMax       = 24;
    skill.mCurrent   = 24;
    skill.mTrueSkill = 24;

    mSkills.ToggleSkill(BAK::SkillType::Lockpick);
    mSkills.ImproveSkill(
            BAK::SkillType::Lockpick, 
            BAK::SkillChange::Direct,
            5 << 8);

    EXPECT_EQ(skill.mTrueSkill, 31);
    EXPECT_EQ(skill.mExperience, 128);
    EXPECT_EQ(skill.mUnseenImprovement, true);
}

TEST_F(SkillTestFixture, DoAdjustHealth_FullHeal)
{
    mSkills.SetSkill(BAK::SkillType::Health, Skill{0x28, 1, 1, 0, 0, false, false});
    mSkills.SetSkill(BAK::SkillType::Stamina, Skill{0x2d, 0, 0, 0, 0, false, false});
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 1);
    DoAdjustHealth(mSkills, mConditions, 100, 0x7fff);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0x28);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0x2d);
}

TEST_F(SkillTestFixture, DoAdjustHealth_FullHealWithNearDeath)
{
    mConditions.IncreaseCondition(BAK::Condition::NearDeath, 100);
    mSkills.SetSkill(BAK::SkillType::Health, Skill{0x28, 0x0, 0x0, 0, 0, false, false});
    mSkills.SetSkill(BAK::SkillType::Stamina, Skill{0x2d, 0x0, 0x0, 0, 0, false, false});
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0);
    DoAdjustHealth(mSkills, mConditions, 100, 0x7fff);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0x1);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0);
}

TEST_F(SkillTestFixture, DoAdjustHealth_ReduceHealth)
{
    mSkills.SetSkill(BAK::SkillType::Health, Skill{0x28, 0x28, 0x28, 0, 0, false, false});
    mSkills.SetSkill(BAK::SkillType::Stamina, Skill{0x2d, 0x28, 0x28, 0, 0, false, false});
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0x28);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0x28);

    // Percentage is irrelevant when reducing health...
    DoAdjustHealth(mSkills, mConditions, 0, -1 * (0x2d << 8));
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0x23);

    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0);
}

TEST_F(SkillTestFixture, DoAdjustHealth_ReduceHealthToNearDeath)
{
    mSkills.SetSkill(BAK::SkillType::Health, Skill{0x28, 0x28, 0x28, 0, 0, false, false});
    mSkills.SetSkill(BAK::SkillType::Stamina, Skill{0x2d, 0x28, 0x28, 0, 0, false, false});
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0x28);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0x28);

    // Percentage is irrelevant when reducing health...
    DoAdjustHealth(mSkills, mConditions, 0, -1 * (0xff << 8));
    EXPECT_EQ(mSkills.GetSkill(SkillType::Health).mTrueSkill, 0x0);
    EXPECT_EQ(mSkills.GetSkill(SkillType::Stamina).mTrueSkill, 0x0);
    EXPECT_EQ(mConditions.GetCondition(Condition::NearDeath).Get(), 100);
}

}
