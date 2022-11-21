#include "gtest/gtest.h"

#include "bak/lock.hpp"
#include "bak/inventoryItem.hpp"

#include "com/logger.hpp"

namespace BAK {

struct LockTestFixture : public ::testing::Test
{
    LockTestFixture()
    :
        mKeyObject{
            "Key",
            1, 1, 1,
            1, 1, 1, 1,
            0, 1, 0, 0, 0, 0,
            RacialModifier::None,
            0,
            ItemType::Key,
            0, 0, 0, 0, 0,
            0, 0, 0
        }
    {
    }

    InventoryItem MakeKey(unsigned item)
    {
        return InventoryItem{
            &mKeyObject,
            ItemIndex{item},
            1,
            0,
            0};
    }

protected:
    void SetUp() override
    {
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    }

    GameObject mKeyObject;
};

TEST_F(LockTestFixture, TryOpenLockWithKey)
{
    auto key = MakeKey(62);
    EXPECT_EQ(TryOpenLockWithKey(key, 90), true);
}

TEST(FairyChest, ParseData)
{
    const auto data = std::string{
R"(DAY NIGHT
#
DRV SAGHO
OAB FIRMT
FLC HRUNI
EPY NGHSK
#
The light one breaks but never falls.
His brother falls but never breaks.)"};
    const auto fairyChest = GenerateFairyChest(data);
    EXPECT_EQ(fairyChest.mAnswer, "DAY NIGHT");
    ASSERT_EQ(fairyChest.mOptions.size(), 4);
    EXPECT_EQ(fairyChest.mOptions[0], "DRV SAGHO");
    EXPECT_EQ(fairyChest.mOptions[1], "OAB FIRMT");
    EXPECT_EQ(fairyChest.mOptions[2], "FLC HRUNI");
    EXPECT_EQ(fairyChest.mOptions[3], "EPY NGHSK");
    EXPECT_EQ(fairyChest.mHint, "\nThe light one breaks but never falls.\nHis brother falls but never breaks.");
}

}
