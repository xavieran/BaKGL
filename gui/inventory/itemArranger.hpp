#pragma once

#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/dragEndpoint.hpp"
#include "gui/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class Grid
{
public:
    Grid(
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

    void Occupy(
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

    glm::vec<2, unsigned> GetNextSlot() const
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

    const auto& GetGrid() const
    {
        return mGrid;
    }

    bool Get(unsigned column, unsigned row) const
    {
        ASSERT(row < mRows);
        ASSERT(column < mColumns);
        return mGrid[row][column];
    }

private:
    unsigned mColumns;
    unsigned mRows;

    std::vector<std::vector<bool>> mGrid;
};

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

class ItemArranger
{
public:
    ItemArranger()
    :
        mLogger{Logging::LogState::GetLogger("Gui::Inventory::ItemArranger")}
    {
    }

    template <typename It, typename AddItem>
    void PlaceItems(
        const It& begin,
        const It& end,
        unsigned columns,
        unsigned rows,
        glm::vec2 slotDims,
        bool homogenousSlots,
        AddItem&& addItem)
    {
        const auto GetSlotDims = [&](const auto& item){
            if (homogenousSlots)
                return slotDims;

            switch (item.GetObject().mImageSize)
            {
            case 1:
                return slotDims;
            case 2:
                return slotDims * glm::vec2{2, 1};
            case 4: 
                return slotDims * glm::vec2{2, 2};
            default:
                ASSERT(false);
                return slotDims;
            }
        };

        auto grid = Grid{columns, rows};

        const auto UpdateNextSlot = [&](const auto& item)
        {
            if (homogenousSlots)
            {
                grid.Occupy(1, 1);
            }
            else
            {
                switch (item.GetObject().mImageSize)
                {
                case 1:
                    grid.Occupy(1, 1); break;
                case 2:
                    grid.Occupy(2, 1); break;
                case 4: 
                    grid.Occupy(2, 2); break;
                default:
                    ASSERT(false);
                    break;
                }
            }

        };

        for (auto it = begin; it != end; it++)
        {
            ASSERT(it != end);
            const auto& [invIndex, itemPtr] = *it;

            ASSERT(itemPtr);
            const auto& item = *itemPtr;

            const auto slot = grid.GetNextSlot();
            const auto itemPos = glm::cast<float>(slot) * slotDims;

            mLogger.Spam() << "Item: " << item << " slot: " << slot << "\n";
            UpdateNextSlot(item);
            mLogger.Spam() << "Grid:\n" << grid << "\n";

            addItem(invIndex, item, itemPos, GetSlotDims(item));
        }
    }

    const Logging::Logger& mLogger;
};

}
