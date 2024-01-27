#include "gtest/gtest.h"

#include "bak/character.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

namespace BAK {

struct CharacterTestFixture : public ::testing::Test
{
    CharacterTestFixture()
    :
        mObjects{}
    {
        mObjects.emplace_back(GameObject{
            .mName = "Normal",
            .mImageSize = 1
            });

        mObjects.emplace_back(GameObject{
            .mName = "Sword",
            .mImageSize = 2,
            .mType = ItemType::Sword});

        mObjects.emplace_back(GameObject{
            .mName = "Armor",
            .mImageSize = 4,
            .mType = ItemType::Armor,
            });

        mObjects.emplace_back(GameObject{
            .mName = "Stack",
            .mFlags = 0x0800,
            .mImageSize = 1,
            .mStackSize = mStackSize,
            .mDefaultStackSize = 5
            });
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

TEST(CharacterTest, CheckAndSetStatus)
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

}  // namespace
