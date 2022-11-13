#include "gtest/gtest.h"

#include "bak/objectInfo.hpp"
#include "bak/temple.hpp"

#include "com/logger.hpp"

namespace BAK {

struct TempleTestFixture : public ::testing::Test
{
    TempleTestFixture()
    :
        mObjects{}
    {
        mObjects.emplace_back(GameObject{
            "Staff",
            1, 1, 1,
            5, -5, 12, 10,
            4, 4, 0, 0,
            RacialModifier::None,
            ItemType::Other,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );
        mObjects.emplace_back(GameObject{
            "Sword",
            1008, 1, 10000,
            71, 0, 49, 10,
            28, 2, 0, 0,
            RacialModifier::Human,
            ItemType::Sword,
            0, 0, 0, 0, 0,
            3, 3, 50}
        );
        mObjects.emplace_back(GameObject{
            "Armor",
            1, 1, 5000,
            1, 1, 1, 1,
            0, 4, 0, 0,
            RacialModifier::None,
            ItemType::Armor,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );
    }

    auto GetObject(const std::string& object)
    {
        return std::find_if(mObjects.begin(), mObjects.end(),
            [&object](const auto& elem){
                return elem.mName == object;
            });
    }

    InventoryItem MakeItem(const std::string& item, auto condition)
    {
        auto it = GetObject(item);
        ASSERT(it != mObjects.end());
        return InventoryItem{
            &(*it),
            ItemIndex{
                static_cast<std::uint8_t>(
                    std::distance(mObjects.begin(), it))},
            static_cast<std::uint8_t>(condition),
            0,
            0};
    }

protected:
    void SetUp() override
    {
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    }

    static constexpr auto mStackSize = 10;
    std::vector<GameObject> mObjects;
};

TEST_F(TempleTestFixture, CanBless)
{
    EXPECT_EQ(Temple::CanBlessItem(MakeItem("Staff", 1)), false);
    EXPECT_EQ(Temple::CanBlessItem(MakeItem("Sword", 1)), true);
    EXPECT_EQ(Temple::CanBlessItem(MakeItem("Armor", 1)), true);
}

TEST_F(TempleTestFixture, IsBlessed)
{
    auto item = MakeItem("Sword", 1);
    EXPECT_EQ(Temple::IsBlessed(item), false);
    item.SetModifier(Modifier::Blessing1);
    EXPECT_EQ(Temple::IsBlessed(item), true);
    item.UnsetModifier(Modifier::Blessing1);
    EXPECT_EQ(Temple::IsBlessed(item), false);
}

TEST_F(TempleTestFixture, BlessItem)
{
    const auto shopStats = ShopStats{
        0, 25, 75, 3, 0, 0, 0, 0, 0, std::array<std::uint8_t, 3>{0, 0, 0,}, 0, 0, 0
    };
    auto item = MakeItem("Sword", 1);
    EXPECT_EQ(Temple::IsBlessed(item), false);
    Temple::BlessItem(item, shopStats);
    EXPECT_EQ(item.HasModifier(Modifier::Blessing3), true);
    Temple::RemoveBlessing(item);
    EXPECT_EQ(item.HasModifier(Modifier::Blessing3), false);
}

TEST_F(TempleTestFixture, CalculateBlessPrice)
{
    const auto shopStats = ShopStats{
        0, 25, 75, 1, 0, 0, 0, 0, 0, std::array<std::uint8_t, 3>{0, 0, 0,}, 0, 0, 0
    };

    EXPECT_EQ(
        Temple::CalculateBlessPrice(MakeItem("Sword", 1), shopStats),
        Royals{7750});
}

TEST_F(TempleTestFixture, CalculateCureCost)
{
    auto conditions = Conditions{};
    auto skills = Skills{};
    conditions.IncreaseCondition(BAK::Condition::NearDeath, 100);

    EXPECT_EQ(
        Temple::CalculateCureCost(65, false, skills, conditions),
        Royals{1956});
}
}
