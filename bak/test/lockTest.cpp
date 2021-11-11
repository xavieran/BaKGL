#include "gtest/gtest.h"

#include "bak/lock.hpp"
#include "bak/inventoryItem.hpp"

namespace BAK {

struct LockTestFixture : public ::testing::Test
{
    LockTestFixture()
    :
        mKeyObject{
            "Key",
            1, 1, 1,
            1, 1, 1, 1,
            0, 1, 0, 0,
            RacialModifier::None,
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

}
