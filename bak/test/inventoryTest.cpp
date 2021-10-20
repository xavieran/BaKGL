#include "gtest/gtest.h"

#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

namespace BAK {

struct InventoryTestFixture : public ::testing::Test
{
    InventoryTestFixture()
    :
        mSwordObject{
            "Sword",
            1, 1, 1,
            1, 1, 1, 1,
            0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Sword,
            0, 0, 0, 0
        },
        mSwordItem{
            &mSwordObject,
            ItemIndex{0},
            99,
            0,
            0
        }
    {}

protected:
    void SetUp() override
    {
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    }

    GameObject mSwordObject;
    InventoryItem mSwordItem;
};

TEST(InventoryTest, CheckAndSetStatus)
{
    {
        std::uint8_t status = 0;
        EXPECT_EQ(CheckItemStatus(status, ItemStatus::Equipped), false);
        status = 1 << static_cast<std::uint8_t>(ItemStatus::Equipped);
        EXPECT_EQ(CheckItemStatus(status, ItemStatus::Equipped), true);
    }

    {
        std::uint8_t status = 0;
        EXPECT_EQ(CheckItemStatus(status, ItemStatus::Equipped), false);

        status = SetItemStatus(status, ItemStatus::Equipped, true);
        ASSERT_EQ(CheckItemStatus(status, ItemStatus::Equipped), true);

        status = SetItemStatus(status, ItemStatus::Equipped, false);
        EXPECT_EQ(CheckItemStatus(status, ItemStatus::Equipped), false);
    }
}

TEST_F(InventoryTestFixture, GetAndSetEquipped)
{
    EXPECT_EQ(mSwordItem.IsEquipped(), false);
    mSwordItem.SetEquipped(true);
    EXPECT_EQ(mSwordItem.IsEquipped(), true);
    mSwordItem.SetEquipped(false);
    EXPECT_EQ(mSwordItem.IsEquipped(), false);
}

TEST_F(InventoryTestFixture, FindEquipped)
{
    auto inventory = Inventory{};
    inventory.AddItem(
        InventoryItem{
            &mSwordObject,
            ItemIndex{0},
            99,
            0,
            0});

    auto it = inventory.FindEquipped(ItemType::Sword);
    EXPECT_EQ(it, inventory.GetItems().end());
    
}


}  // namespace
