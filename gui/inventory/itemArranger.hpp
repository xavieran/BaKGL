#pragma once

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

namespace Gui {

class Grid
{
public:
    Grid(
        unsigned columns,
        unsigned rows);

    void Occupy(
        unsigned columns,
        unsigned rows);

    glm::vec<2, unsigned> GetNextSlot() const;

    const auto& GetGrid() const;
    bool Get(unsigned column, unsigned row) const;

private:
    unsigned mColumns;
    unsigned mRows;

    std::vector<std::vector<bool>> mGrid;
};

std::ostream& operator<<(std::ostream& os, const Grid& grid);

class ItemArranger
{
public:
    ItemArranger();

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
                grid.Occupy(1, 1);
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

private:
    const Logging::Logger& mLogger;
};

}
