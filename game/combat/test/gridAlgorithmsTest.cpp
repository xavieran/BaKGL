#include "gtest/gtest.h"

#include "game/combat/gridAlgorithms.hpp"
#include "game/combat/grid.hpp"
#include "game/combat/types.hpp"
#include "com/bits.hpp"

namespace Game::Combat {

struct GridAlgorithmsTest : public ::testing::Test
{
    GridAlgorithmsTest()
    :   grid{5, 5}
    {
        for (unsigned y = 0; y < grid.GetRows(); y++)
        {
            for (unsigned x = 0; x < grid.GetCols(); x++)
            {
                grid.Get(x, y).mState = SetBit(
                    grid.Get(x, y).mState,
                    StateFlags::Reachable,
                    true);
            }
        }
    }

    void SetUnreachable(int x, int y)
    {
        grid.Get(x, y).mState = SetBit(
            grid.Get(x, y).mState,
            StateFlags::Reachable,
            false);
    }

    Grid grid;
};

TEST_F(GridAlgorithmsTest, ChebyshevDistance_Basic)
{
    EXPECT_EQ(ChebyshevDistance({0, 0}, {3, 0}), 3u);
    EXPECT_EQ(ChebyshevDistance({0, 0}, {0, 3}), 3u);
    EXPECT_EQ(ChebyshevDistance({0, 0}, {3, 4}), 4u);
    EXPECT_EQ(ChebyshevDistance({1, 2}, {1, 2}), 0u);
}

TEST_F(GridAlgorithmsTest, CalculatePath_FindsPath)
{
    auto path = CalculatePath({0, 0}, {4, 4}, grid);
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.back(), GridPos(4, 4));
    EXPECT_EQ(path.front(), GridPos(1, 1));
}

TEST_F(GridAlgorithmsTest, CalculatePath_Blocked)
{
    SetUnreachable(1, 1);
    SetUnreachable(1, 0);
    SetUnreachable(0, 1);

    auto path = CalculatePath({0, 0}, {2, 2}, grid);
    EXPECT_TRUE(path.empty());
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_WhenAdjacentReturnCurrentPosition)
{
    // # # #
    // . # #
    // # # #
    auto result = SelectBestAttackPosition({0, 1}, {1, 1}, grid);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, GridPos(0, 1));
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_WhenAllCardinalsBlockedReturnNull)
{
    // # x #
    // x # x -> unreachable
    // # x #
    auto target = glm::ivec2{1, 1};
    SetUnreachable(0, 1);
    SetUnreachable(1, 0);
    SetUnreachable(2, 1);
    SetUnreachable(1, 2);
    std::cout << grid << "\n";

    auto result = SelectBestAttackPosition({0, 0}, target, grid);
    EXPECT_FALSE(result);
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_ReturnOnlyOption)
{
    // # # .
    // # x #
    // x # x -> unreachable
    // # # #
    auto src = glm::ivec2{2, 3};
    auto target = glm::ivec2{1, 1};
    SetUnreachable(0, 1);
    SetUnreachable(2, 1);
    SetUnreachable(1, 2);
    std::cout << grid << "\n";

    auto result = SelectBestAttackPosition(src, target, grid);
    ASSERT_TRUE(result);
    EXPECT_EQ((*result), GridPos(1, 0));
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_ReturnClosestOption)
{
    // # # . 
    // # o #
    // x t # 
    // # x #
    auto src = glm::ivec2{2, 3};
    auto target = glm::ivec2{1, 1};
    SetUnreachable(0, 1);
    SetUnreachable(1, 0);
    std::cout << grid << "\n";

    auto result = SelectBestAttackPosition(src, target, grid);
    ASSERT_TRUE(result);
    EXPECT_EQ((*result), GridPos(1, 2));
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_ReturnCardinalWhenDiagonal)
{
    // # # # 
    // # # #
    // x t o 
    // # x .
    auto src = glm::ivec2{2, 0};
    auto target = glm::ivec2{1, 1};
    SetUnreachable(0, 1);
    SetUnreachable(1, 0);
    std::cout << grid << "\n";

    auto result = SelectBestAttackPosition(src, target, grid);
    ASSERT_TRUE(result);
    EXPECT_EQ((*result), GridPos(2, 1));
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_ReturnLongWayRoundOption)
{
    // # # #
    // # # # 
    // # x #
    // o t x 
    // # # #
    auto src = glm::ivec2{2, 0};
    auto target = glm::ivec2{1, 1};
    SetUnreachable(0, 1);
    SetUnreachable(1, 0);
    std::cout << grid << "\n";

    auto result = SelectBestAttackPosition(src, target, grid);
    ASSERT_TRUE(result);
    EXPECT_EQ((*result), GridPos(2, 1));
}

TEST_F(GridAlgorithmsTest, SelectBestAttackPosition_RealisticScenario)
{
    // 1 1 1 1 1 1 1 1
    // 1 1 t 1 1 1 1 1
    // 1 1 o 1 1 1 1 1
    // 1 1 1 1 1 1 1 1
    // 1 1 1 1 1 1 1 1
    // 1 1 1 1 1 1 1 1
    // . 1 1 1 1 1 1 1
    // 1 1 1 1 1 1 1 1
    grid = Grid{8, 8};
    for (unsigned y = 0; y < grid.GetRows(); y++)
        for (unsigned x = 0; x < grid.GetCols(); x++)
            grid.Get(x, y).mState = SetBit(
                grid.Get(x, y).mState, StateFlags::Reachable, true);

    auto target = GridPos{2, 6};
    grid.Get(target).mState = SetBit(grid.Get(target).mState, StateFlags::Reachable, false);

    std::cout << grid;

    auto result = SelectBestAttackPosition({0, 1}, target, grid);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, GridPos(2, 5));
}
}
