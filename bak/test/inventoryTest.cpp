#include "gtest/gtest.h"

#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include "com/logger.hpp"

namespace BAK {

struct InventoryTestFixture : public ::testing::Test
{
    InventoryTestFixture()
    :
        mObjects{}
    {
        mObjects.emplace_back(GameObject{
            "Normal",
            1, 1, 1,
            1, 1, 1, 1,
            0, 1, 0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Other,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );

        mObjects.emplace_back(GameObject{
            "Sword",
            1, 1, 1,
            1, 1, 1, 1,
            0, 2, 0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Sword,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );
        mObjects.emplace_back(GameObject{
            "Armor",
            1, 1, 1,
            1, 1, 1, 1,
            0, 4, 0, 0, 0, 0,
            RacialModifier::None,
            ItemType::Armor,
            0, 0, 0, 0, 0,
            0, 0, 0}
        );
        mObjects.emplace_back(GameObject{
            "Stack",
            0x0800, 1, 1,
            1, 1, 1, 1,
            0, 1, 0, 0, mStackSize, 5,
            RacialModifier::None,
            ItemType::Sword,
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
    auto swordItem = MakeItem("Sword", 99);
    EXPECT_EQ(swordItem.IsEquipped(), false);
    swordItem.SetEquipped(true);
    EXPECT_EQ(swordItem.IsEquipped(), true);
    swordItem.SetEquipped(false);
    EXPECT_EQ(swordItem.IsEquipped(), false);
}

TEST_F(InventoryTestFixture, FindEquipped)
{
    auto inventory = Inventory{24};
    inventory.AddItem(
        MakeItem("Sword", 99));

    auto it = inventory.FindEquipped(ItemType::Sword);
    EXPECT_EQ(it, inventory.GetItems().end());
}

TEST_F(InventoryTestFixture, SpaceUsed)
{
    auto inventory = Inventory{
        4,
        {
            MakeItem("Armor", 99),
            MakeItem("Normal", 10),
            MakeItem("Sword", 7),
        }};

    EXPECT_EQ(inventory.GetSpaceUsed(), 4 + 1 + 2);
}

TEST_F(InventoryTestFixture, HasIncompleteStack)
{
    {
        auto inventory = Inventory{
            5,
            {
                MakeItem("Armor", 99),
                MakeItem("Stack", 1),
            }};

        EXPECT_EQ(inventory.HasIncompleteStack(MakeItem("Stack", 1)), true);
    }

    {
        auto inventory = Inventory{
            5,
            {
                MakeItem("Armor", 99),
                MakeItem("Stack", mStackSize),
            }};

        EXPECT_EQ(inventory.HasIncompleteStack(MakeItem("Stack", 1)), false);
    }
}

TEST_F(InventoryTestFixture, FindStack)
{
    {
        auto inventory = Inventory{
            5,
            {
                MakeItem("Stack", 10),
                MakeItem("Stack", 6),
            }};

        const auto it = inventory.FindStack(MakeItem("Stack", 1));
        ASSERT_NE(it, inventory.GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 6);
    }

    {
        auto inventory = Inventory{
            5,
            {
                MakeItem("Stack", 10)
            }};

        const auto it = inventory.FindStack(MakeItem("Stack", 1));
        ASSERT_NE(it, inventory.GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 10);
    }

    {
        auto inventory = Inventory{
            5,
            {
                MakeItem("Armor", 99),
                MakeItem("Stack", 10),
                MakeItem("Stack", 6),
            }};

        const auto it = inventory.FindStack(MakeItem("Stack", 1));
        ASSERT_NE(it, inventory.GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 6);
    }

    {
        auto inventory = Inventory{
            10,
            {
                MakeItem("Armor", 99),
                MakeItem("Stack", 10),
                MakeItem("Stack", 6),
                MakeItem("Stack", 10),
                MakeItem("Stack", 4),
                MakeItem("Stack", 6),
                MakeItem("Stack", 10),
            }};

        const auto it = inventory.FindStack(MakeItem("Stack", 1));
        ASSERT_NE(it, inventory.GetItems().end());
        EXPECT_EQ(it->GetQuantity(), 4);
    }

    {
        auto inventory = Inventory{
            5,
            {
                MakeItem("Armor", 99),
            }};

        const auto it = inventory.FindStack(MakeItem("Stack", 1));
        EXPECT_EQ(it, inventory.GetItems().end());
    }
}

TEST_F(InventoryTestFixture, CanAddCharacterSuccess)
{
    auto inventory = Inventory{4};

    auto armor = MakeItem("Armor", 99);
    auto sword = MakeItem("Sword", 99);
    auto stack = MakeItem("Stack", 5);

    EXPECT_EQ(inventory.CanAddCharacter(armor), 1);
    EXPECT_EQ(inventory.CanAddCharacter(sword), 1);
    EXPECT_EQ(inventory.CanAddCharacter(stack), 5);
}


TEST_F(InventoryTestFixture, CanAddCharacterPartial)
{
    auto inventory = Inventory{
        5,
        {
            MakeItem("Armor", 99),
            MakeItem("Stack", 1),
        }};

    EXPECT_EQ(inventory.GetSpaceUsed(), 4 + 1);

    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Normal", 1)), 0);
    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Sword", 1)), 0);
    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Armor", 1)), 0);

    auto stack5 = MakeItem("Stack", 5);
    auto stack10 = MakeItem("Stack", 10);

    EXPECT_EQ(inventory.CanAddCharacter(stack5), 5);
    EXPECT_EQ(inventory.CanAddCharacter(stack10), 9);
}

TEST_F(InventoryTestFixture, CanAddCharacterFail)
{
    auto inventory = Inventory{
        5,
        {
            MakeItem("Armor", 99),
            MakeItem("Stack", 10),
        }};

    EXPECT_EQ(inventory.GetSpaceUsed(), 4 + 1);

    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Normal", 1)), 0);
    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Sword", 1)), 0);
    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Armor", 1)), 0);

    auto stack5 = MakeItem("Stack", 5);
    auto stack10 = MakeItem("Stack", 10);

    EXPECT_EQ(inventory.CanAddCharacter(stack5), 0);
    EXPECT_EQ(inventory.CanAddCharacter(stack10), 0);
}

TEST_F(InventoryTestFixture, CanAddContainerFail)
{
    auto inventory = Inventory{
        2,
        {
            MakeItem("Armor", 99),
            MakeItem("Stack", 10),
        }};

    EXPECT_EQ(inventory.GetSpaceUsed(), 4 + 1);

    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Normal", 1)), 0);
    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Sword", 1)), 0);
    EXPECT_EQ(inventory.CanAddCharacter(MakeItem("Armor", 1)), 0);

    auto stack5 = MakeItem("Stack", 5);
    auto stack10 = MakeItem("Stack", 10);

    EXPECT_EQ(inventory.CanAddCharacter(stack5), 0);
    EXPECT_EQ(inventory.CanAddCharacter(stack10), 0);
}

 
TEST_F(InventoryTestFixture, AddItem)
{
    auto inventory = Inventory{
        5,
        {
        }};
    inventory.AddItem(MakeItem("Armor", 1));

    ASSERT_EQ(inventory.GetNumberItems(), 1);

    {
        const auto& added = inventory.GetAtIndex(InventoryIndex{0});
        EXPECT_EQ(added.GetCondition(), 1);
    }

    inventory.AddItem(MakeItem("Stack", mStackSize));

    ASSERT_EQ(inventory.GetNumberItems(), 2);

    {
        const auto& added = inventory.GetAtIndex(InventoryIndex{1});
        EXPECT_EQ(added.GetQuantity(), mStackSize);
    }

    inventory.AddItem(MakeItem("Stack", 4));

    ASSERT_EQ(inventory.GetNumberItems(), 3);

    {
        const auto& added = inventory.GetAtIndex(InventoryIndex{2});
        EXPECT_EQ(added.GetQuantity(), 4);
    }

    inventory.AddItem(MakeItem("Stack", 3));

    ASSERT_EQ(inventory.GetNumberItems(), 3);

    {
        const auto& added = inventory.GetAtIndex(InventoryIndex{2});
        EXPECT_EQ(added.GetQuantity(), 7);
    }

    inventory.AddItem(MakeItem("Stack", 6));

    ASSERT_EQ(inventory.GetNumberItems(), 4);

    {
        const auto& existing = inventory.GetAtIndex(InventoryIndex{2});
        EXPECT_EQ(existing.GetQuantity(), mStackSize);

        const auto& added = inventory.GetAtIndex(InventoryIndex{3});
        EXPECT_EQ(added.GetQuantity(), 3);
    }

    inventory.AddItem(MakeItem("Stack", mStackSize));

    ASSERT_EQ(inventory.GetNumberItems(), 5);

    {
        const auto& existing = inventory.GetAtIndex(InventoryIndex{3});
        EXPECT_EQ(existing.GetQuantity(), mStackSize);

        const auto& added = inventory.GetAtIndex(InventoryIndex{4});
        EXPECT_EQ(added.GetQuantity(), 3);
    }
}

TEST_F(InventoryTestFixture, RemoveItem)
{
    {
        auto inventory = Inventory{
            5,
            {
            }};
        inventory.AddItem(MakeItem("Armor", 1));

        ASSERT_EQ(inventory.GetNumberItems(), 1);

        // This overload will just remove the first item that matches type
        inventory.RemoveItem(MakeItem("Armor", 5));
        ASSERT_EQ(inventory.GetNumberItems(), 0);
    }

    {
        auto inventory = Inventory{
            5,
            {
            }};
        inventory.AddItem(MakeItem("Armor", 1));

        ASSERT_EQ(inventory.GetNumberItems(), 1);

        // This overload removes the matching index
        inventory.RemoveItem(InventoryIndex{0});
        ASSERT_EQ(inventory.GetNumberItems(), 0);
    }

    {
        auto inventory = Inventory{
            5,
            {
            }};
        inventory.AddItem(MakeItem("Stack", 10));

        ASSERT_EQ(inventory.GetNumberItems(), 1);

        // This overload removes the matching index, even for stackables
        inventory.RemoveItem(InventoryIndex{0});
        ASSERT_EQ(inventory.GetNumberItems(), 0);
    }

    {
        auto inventory = Inventory{
            5,
            {
            }};
        inventory.AddItem(MakeItem("Stack", 10));

        ASSERT_EQ(inventory.GetNumberItems(), 1);

        // Remove the whole stack
        inventory.RemoveItem(MakeItem("Stack", 10));
        ASSERT_EQ(inventory.GetNumberItems(), 0);
    }

    {
        auto inventory = Inventory{
            5,
            {
            }};
        inventory.AddItem(MakeItem("Stack", 10));

        ASSERT_EQ(inventory.GetNumberItems(), 1);

        // Partially remove from the stack
        inventory.RemoveItem(MakeItem("Stack", 7));
        ASSERT_EQ(inventory.GetNumberItems(), 1);
        EXPECT_EQ(inventory.GetAtIndex(InventoryIndex{0}).GetQuantity(), 3);
    }

    {
        auto inventory = Inventory{
            5,
            {
            }};
        inventory.AddItem(MakeItem("Stack", 10));
        inventory.AddItem(MakeItem("Stack", 6));

        ASSERT_EQ(inventory.GetNumberItems(), 2);

        // Partially remove from the 1st stack and completely remove the 2nd
        inventory.RemoveItem(MakeItem("Stack", 8));
        ASSERT_EQ(inventory.GetNumberItems(), 1);
        EXPECT_EQ(inventory.GetAtIndex(InventoryIndex{0}).GetQuantity(), 8);
    }
}

}  // namespace
