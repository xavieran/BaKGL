#include "gtest/gtest.h"

#include "com/logger.hpp"
#include "gui/inventory/itemArranger.hpp"

#include <glm/glm.hpp>

namespace Gui::Test {

TEST(GridTest, FillWithSize1)
{
    const auto& logger = Logging::LogState::GetLogger("GridTest");
    const auto rows = 4;
    const auto columns = 5;

    auto grid = Grid{columns, rows};
    unsigned row = 0;
    unsigned column = 0;

    for (unsigned i = 0; i < rows * columns; i++)
    {
        EXPECT_EQ(
            grid.GetNextSlot(),
            (glm::vec<2, unsigned>{column, row}));

        EXPECT_EQ(grid.Get(column, row), false);

        grid.Occupy(1, 1);

        EXPECT_EQ(grid.Get(column, row), true);

        if ((++row) == rows)
        {
            row = 0;
            column++;
        }
    }
}

}  // namespace
