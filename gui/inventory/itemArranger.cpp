#include "gui/inventory/itemArranger.hpp"

namespace Gui {

Grid::Grid(
    unsigned columns,
    unsigned rows)
:
    mColumns{columns},
    mRows{rows},
    mGrid{std::invoke([&]{
        std::vector<std::vector<bool>> grid;
        for (unsigned i = 0; i < rows; i++)
            grid.emplace_back(columns, false);
        return grid;
    })}
{
}

void Grid::Occupy(
    unsigned columns,
    unsigned rows)
{
    const auto& logger = Logging::LogState::GetLogger("Grid");
    const auto currentPos = GetNextSlot();
    for (unsigned r = 0; r < rows; r++)
    {
        for (unsigned c = 0; c < columns; c++)
        {
            ASSERT(currentPos.y + r < mRows);
            ASSERT(currentPos.x + c < mColumns);
            mGrid[currentPos.y + r][currentPos.x + c] = true;
        }
    }
}

glm::vec<2, unsigned> Grid::GetNextSlot() const
{
    const auto& logger = Logging::LogState::GetLogger("Grid");
    std::optional<glm::vec<2, unsigned>> slot{};
    unsigned r = 0;
    unsigned c = 0;
    while (!slot)
    {
        if (!Get(c, r))
            slot = glm::vec<2, unsigned>{c, r};

        if ((++r) == mRows)
        {
            r = 0;
            c++;
        }
    }

    ASSERT(slot);
    return *slot;
}

const auto& Grid::GetGrid() const
{
    return mGrid;
}

bool Grid::Get(unsigned column, unsigned row) const
{
    ASSERT(row < mRows);
    ASSERT(column < mColumns);
    return mGrid[row][column];
}

std::ostream& operator<<(std::ostream& os, const Grid& grid)
{
    for (const auto& row : grid.GetGrid())
    {
        for (const auto c : row)
        {
            os << (c ? '*' : '.');
        }
        os << '\n';
    }

    return os;
}

ItemArranger::ItemArranger()
:
    mLogger{Logging::LogState::GetLogger("Gui::Inventory::ItemArranger")}
{
}

}
