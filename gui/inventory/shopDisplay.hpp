#pragma once

#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/gameState.hpp"
#include "bak/objectInfo.hpp"
#include "bak/shop.hpp"
#include "bak/state/money.hpp"

#include "gui/inventory/equipmentSlot.hpp"
#include "gui/inventory/itemArranger.hpp"
#include "gui/inventory/shopItemSlot.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/core/dragEndpoint.hpp"
#include "gui/core/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class ShopDisplay :
    public Widget
{
public:
    static constexpr auto mItemsPerPage = 6;

    ShopDisplay(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font,
        const BAK::GameState& gameState,
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
        mShopPage{0},
        mInventoryItems{},
        mDiscount{},
        mContainer{nullptr},
        mGameState(gameState),
        mShowDescription{std::move(showDescription)},
        mLogger{Logging::LogState::GetLogger("Gui::ShopDisplay")}
    {
        assert(mShowDescription);
    }

    void SetContainer(BAK::IContainer* container)
    {
        ASSERT(container);
        mContainer = container;
        mShopPage = 0;
        ClearDiscounts();
    }

    void RefreshGui()
    {
        ClearChildren();
        UpdateInventoryContents();
        AddChildren();
    }

    void AdvanceNextPage()
    {
        if ((++mShopPage) == GetMaxPages())
            mShopPage = 0;
    }

    std::size_t GetMaxPages()
    {
        ASSERT(mContainer);
        const auto nItems = mContainer->GetInventory().GetNumberItems();
        const auto fullPages = nItems / mItemsPerPage;
        const auto partialPages = (nItems % mItemsPerPage) != 0;
        return fullPages + partialPages;
    }

    BAK::Royals GetSellPrice(
        BAK::InventoryIndex itemIndex,
        unsigned amount)
    {
        ASSERT(mContainer);
        auto item = mContainer->GetInventory().GetAtIndex(itemIndex);
        item.SetQuantity(amount);
        return BAK::Shop::GetSellPrice(
            item,
            mContainer->GetShop(),
            mDiscount[item.GetItemIndex()],
            BAK::State::IsRomneyGuildWars(mGameState, mContainer->GetShop()));
    }

    BAK::Royals GetBuyPrice(const BAK::InventoryItem& item) const
    {
        ASSERT(mContainer);
        return BAK::Shop::GetBuyPrice(item, mContainer->GetShop(), BAK::State::IsRomneyGuildWars(mGameState, mContainer->GetShop()));
    }

    bool CanBuyItem(const BAK::InventoryItem& item) const
    {
        ASSERT(mContainer);
        return BAK::Shop::CanBuyItem(item, *mContainer);
    }

    void SetItemDiscount(BAK::InventoryIndex itemIndex, BAK::Royals discount)
    {
        mLogger.Debug() << " Setting discount to: " << discount << "\n";
        ASSERT(mContainer);
        const auto& item = mContainer->GetInventory().GetAtIndex(itemIndex);
        mDiscount[item.GetItemIndex()] = discount;
    }

    BAK::Royals GetDiscount(BAK::InventoryIndex itemIndex)
    {
        ASSERT(mContainer);
        const auto& item = mContainer->GetInventory().GetAtIndex(itemIndex);
        return mDiscount[item.GetItemIndex()];
    }

private:

    void ShowItemDescription(const BAK::InventoryItem& item)
    {
        mShowDescription(item);
    }

    void ClearDiscounts()
    {
        mDiscount.clear();
        const auto& inventory = mContainer->GetInventory();
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
            || mContainer->GetContainerType() == BAK::ContainerType::Inn  )
        {
            ASSERT(items.size() > mShopPage * mItemsPerPage);

            const auto begin = std::next(items.begin(), mShopPage * mItemsPerPage);
            const auto nItems = std::distance(begin, items.end());
            const auto end = std::next(
                begin,
                std::min(
                    static_cast<size_t>(mItemsPerPage),
                    static_cast<size_t>(nItems)));

            arranger.PlaceItems(
                begin, end,
                3, 2,
                glm::vec2{98, 60},
                true,
                [&](auto invIndex, const auto& item, const auto itemPos, const auto dims)
                {
                    const auto discount = mDiscount[item.GetItemIndex()];
                    mInventoryItems.emplace_back(
                        itemPos,
                        dims,
                        mFont,
                        mIcons,
                        invIndex,
                        item,
                        BAK::Shop::GetSellPrice(
                            item,
                            mContainer->GetShop(),
                            discount,
                            BAK::State::IsRomneyGuildWars(mGameState, mContainer->GetShop())
                        ),
                        discount != BAK::sUnpurchaseablePrice,
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

    unsigned mShopPage;
    std::vector<DraggableShopItem> mInventoryItems;
    std::unordered_map<BAK::ItemIndex, BAK::Royals> mDiscount;

    BAK::IContainer* mContainer;

    const BAK::GameState& mGameState;
    std::function<void(const BAK::InventoryItem&)> mShowDescription;
    const Logging::Logger& mLogger;
};

}
