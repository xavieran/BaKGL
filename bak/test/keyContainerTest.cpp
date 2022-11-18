#include "gtest/gtest.h"

#include "bak/keyContainer.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include "com/logger.hpp"

namespace BAK {

struct KeyTestFixture : public ::testing::Test
{
    KeyTestFixture()
    :
        mObjects{}
    {
        mObjects.emplace_back(GameObject{
            "Key1",
            1, 1, 1,
            1, 1, 1, 1,
            0, 1, 0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Key,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );

        mObjects.emplace_back(GameObject{
            "Key2",
            1, 1, 1,
            1, 1, 1, 1,
            0, 2, 0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Key,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );

        mObjects.emplace_back(GameObject{
            "NonKey",
            1, 1, 1,
            1, 1, 1, 1,
            0, 2, 0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Other,
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

    std::vector<GameObject> mObjects;
};

TEST_F(KeyTestFixture, CanAddItem)
{
    auto keys = KeyContainer{Inventory{8}};
    EXPECT_EQ(keys.CanAddItem(MakeItem("Key1", 1)), true);
    EXPECT_EQ(keys.CanAddItem(MakeItem("NonKey", 1)), false);
}

TEST_F(KeyTestFixture, GiveItem)
{
    auto keys = KeyContainer{Inventory{8}};
    const auto item = MakeItem("Key1", 1);
    keys.GiveItem(item);

    {
        ASSERT_EQ(keys.GetInventory().GetNumberItems(), 1);
        EXPECT_EQ(keys.GetInventory().HaveItem(item), true);
        auto it = keys.GetInventory().FindItem(item);
        ASSERT_NE(it, keys.GetInventory().GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 1);
    }

    keys.GiveItem(item);

    {
        ASSERT_EQ(keys.GetInventory().GetNumberItems(), 1);
        auto it = keys.GetInventory().FindItem(item);
        ASSERT_NE(it, keys.GetInventory().GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 2);
    }

    const auto item2 = MakeItem("Key2", 1);
    keys.GiveItem(item2);

    {
        ASSERT_EQ(keys.GetInventory().GetNumberItems(), 2);
        EXPECT_EQ(keys.GetInventory().HaveItem(item2), true);
        auto it = keys.GetInventory().FindItem(item2);
        ASSERT_NE(it, keys.GetInventory().GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 1);
    }
}

TEST_F(KeyTestFixture, RemoveItem)
{
    auto item1 = MakeItem("Key1", 1);
    auto item2 = MakeItem("Key2", 1);

    auto keys = KeyContainer{
        Inventory{8, {
            MakeItem("Key1", 4),
            MakeItem("Key2", 1)
        }}};

    keys.RemoveItem(item1);

    {
        ASSERT_EQ(keys.GetInventory().GetNumberItems(), 2);
        EXPECT_EQ(keys.GetInventory().HaveItem(item1), true);
        auto it = keys.GetInventory().FindItem(item1);
        ASSERT_NE(it, keys.GetInventory().GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 3);
    }

    keys.RemoveItem(item1);

    {
        ASSERT_EQ(keys.GetInventory().GetNumberItems(), 2);
        EXPECT_EQ(keys.GetInventory().HaveItem(item1), true);
        auto it = keys.GetInventory().FindItem(item1);
        ASSERT_NE(it, keys.GetInventory().GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 2);
    }

    keys.RemoveItem(item2);

    {
        ASSERT_EQ(keys.GetInventory().GetNumberItems(), 1);
        EXPECT_EQ(keys.GetInventory().HaveItem(item2), false);
    }
}

}  // namespace
