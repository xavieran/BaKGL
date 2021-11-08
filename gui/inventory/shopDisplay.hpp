#pragma once

#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"

#include "gui/inventory/equipmentSlot.hpp"
#include "gui/inventory/itemArranger.hpp"
#include "gui/inventory/shopItemSlot.hpp"

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

using DraggableShopItem = Draggable<ShopItemSlot>;

class ShopDisplay :
    public Widget
{
public:
    ShopDisplay(
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
        mLogger{Logging::LogState::GetLogger("Gui::ShopDisplay")}
    {
        assert(mShowDescription);
    }

    void SetContainer(BAK::IContainer* container)
    {
        ASSERT(container);
        //ASSERT(container->GetContainerType() == BAK::ContainerType::Shop
        //    || container->GetContainerType() == BAK::ContainerType::Inn);
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


        auto arranger = ItemArranger{};
        if (   mContainer->GetContainerType() == BAK::ContainerType::Shop
            || mContainer->GetContainerType() == BAK::ContainerType::Inn)
        {
            arranger.PlaceItems(
                items.begin(), items.begin() + 6,
                3, 2,
                glm::vec2{98, 60},
                true,
                [&](auto invIndex, const auto& item, const auto itemPos, const auto dims)
                {
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
                });
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

    std::vector<DraggableShopItem> mInventoryItems;

    BAK::IContainer* mContainer;

    std::function<void(const BAK::InventoryItem&)> mShowDescription;
    const Logging::Logger& mLogger;
};

}
