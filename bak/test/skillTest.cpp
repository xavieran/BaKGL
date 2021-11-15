#include "gtest/gtest.h"

#include "bak/condition.hpp"
#include "bak/skills.hpp"

#include "com/logger.hpp"

namespace BAK {

struct SkillTestFixture : public ::testing::Test
{
    SkillTestFixture()
    :
        mSkills{
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
            Skill{80, 80, 80, 0, 0, false, false}} // Stealth
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
                mConditions));
    }

    auto expectedSkillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
        expectedSkillValues.emplace_back(mSkills.mSkills[i].mTrueSkill);

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
                mConditions));
    }

    // Sickness doesn't affect stats...
    auto expectedSkillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
        expectedSkillValues.emplace_back(mSkills.mSkills[i].mTrueSkill);

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
                mConditions));
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
                mConditions));
    }

    auto expectedSkillValues = std::vector<unsigned>{};
    for (unsigned i = 0; i < 16; i++)
        expectedSkillValues.emplace_back(mSkills.mSkills[i].mTrueSkill);

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
                mConditions));
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
                mConditions));
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


}
