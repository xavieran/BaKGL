#pragma once

#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"

#include "gui/inventory/equipmentSlot.hpp"
#include "gui/inventory/inventorySlot.hpp"

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

class ContainerDisplay :
    public Widget
{
public:
    ContainerDisplay(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font,
        std::function<void(const BAK::InventoryItem&)>&& showDescription)
    :
        // Black background
        Widget{
            RectTag{},
            pos,
            dims,
            Color::black,
            true
        },
        mFont{font},
        mIcons{icons},
        mInventoryItems{},
        mContainer{nullptr},
        mShowDescription{std::move(showDescription)},
        mLogger{Logging::LogState::GetLogger("Gui::ContainerDisplay")}
    {
        assert(mShowDescription);
    }

    void SetContainer(BAK::IContainer* container)
    {
        ASSERT(container);
        mContainer = container;
    }

    void RefreshGui()
    {
        ClearChildren();
        UpdateInventoryContents();
        AddChildren();
    }

private:

    void ShowItemDescription(const BAK::InventoryItem& item)
    {
        mShowDescription(item);
    }

    void UpdateInventoryContents()
    {
        ASSERT(mContainer != nullptr);
        mInventoryItems.clear();
        const auto& inventory = mContainer->GetInventory();

        std::vector<
            std::pair<
                BAK::InventoryIndex,
            const BAK::InventoryItem*>> items{};

        const auto numItems = inventory.GetItems().size();
        mInventoryItems.reserve(numItems);
        items.reserve(numItems);

        unsigned index{0};
        std::transform(
            inventory.GetItems().begin(), inventory.GetItems().end(),
            std::back_inserter(items),
            [&index](const auto& i) -> std::pair<BAK::InventoryIndex, const BAK::InventoryItem*> {
                return std::make_pair(BAK::InventoryIndex{index++}, &i);
            });

        std::sort(items.begin(), items.end(), [](const auto& l, const auto& r) 
        {
            return (std::get<1>(l)->GetObject().mImageSize 
                > std::get<1>(r)->GetObject().mImageSize);
        });

        unsigned majorColumn = 0;
        unsigned minorColumn = 0;
        unsigned majorRow = 0;
        unsigned minorRow = 0;

        auto pos  = glm::vec2{0, 0};
        const auto slotDims = glm::vec2{40, 29};

        for (const auto& [invIndex, itemPtr] : items)
        {
            ASSERT(itemPtr);
            const auto& item = *itemPtr;
            const auto& [ss, ti, _] = mIcons.GetInventoryIcon(item.mItemIndex.mValue);
            const auto itemPos = pos + glm::vec2{
                    (majorColumn * 2 + minorColumn) * slotDims.x,
                    (majorRow * 2 + minorRow) * slotDims.y};

            auto dims = slotDims;

            mLogger.Spam() << "Item: " << item 
                << " mc: " << minorColumn << " MC: " << majorColumn 
                << " mr: " << minorRow << " MR: " << majorRow << "\n";
            if (item.GetObject().mImageSize == 1)
            {
                minorColumn += 1;
            }
            else if (item.GetObject().mImageSize == 2)
            {
                minorRow += 1;
                dims.x *= 2;
            }
            else if (item.GetObject().mImageSize == 4)
            {
                dims.x *= 2;
                dims.y *= 2;
                majorRow += 1;
            }

            mLogger.Spam() << "AfterPlace: " << item 
                << " mc: " << minorColumn << " MC: " << majorColumn 
                << " mr: " << minorRow << " MR: " << majorRow << "\n";
            mInventoryItems.emplace_back(
                itemPos,
                dims,
                mFont,
                mIcons,
                invIndex,
                item,
                [&]{
                    ShowItemDescription(item);
                });

            if (minorColumn != 0 && minorColumn % 2 == 0)
            {
                minorColumn = 0;
                minorRow += 1;
            }

            if (minorRow != 0 && minorRow % 2 == 0)
            {
                minorRow = 0;
                majorRow += 1;
            }

            if (majorRow != 0 && majorRow % 2 == 0)
            {
                majorColumn += 1;
                majorRow = 0;
            }

            // Handle the final column
            if (majorColumn == 2 && minorColumn > 0)
            {
                minorRow += 1;
                minorColumn = 0;
            }

            mLogger.Spam() << "CorrectRows: " << item 
                << " mc: " << minorColumn << " MC: " << majorColumn 
                << " mr: " << minorRow << " MR: " << majorRow << "\n";
        }
    }

    void AddChildren()
    {
        for (auto& item : mInventoryItems)
            AddChildBack(&item);
    }

private:
    const Font& mFont;
    const Icons& mIcons;

    std::vector<DraggableItem> mInventoryItems;

    BAK::IContainer* mContainer;

    std::function<void(const BAK::InventoryItem&)> mShowDescription;
    const Logging::Logger& mLogger;
};

}
