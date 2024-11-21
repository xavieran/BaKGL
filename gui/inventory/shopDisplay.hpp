#pragma once

#include "bak/types.hpp"

#include "gui/inventory/equipmentSlot.hpp"
#include "gui/inventory/shopItemSlot.hpp"

#include "gui/icons.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class IContainer;
class GameState;
}

namespace Gui {
class Icons;
class Font;

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
        std::function<void(const BAK::InventoryItem&)>&& showDescription);
    void SetContainer(BAK::IContainer* container);
    void RefreshGui();
    void AdvanceNextPage();
    std::size_t GetMaxPages();
    BAK::Royals GetSellPrice(
        BAK::InventoryIndex itemIndex,
        unsigned amount);
    BAK::Royals GetBuyPrice(const BAK::InventoryItem& item) const;
    bool CanBuyItem(const BAK::InventoryItem& item) const;
    void SetItemDiscount(BAK::InventoryIndex itemIndex, BAK::Royals discount);
    BAK::Royals GetDiscount(BAK::InventoryIndex itemIndex);

private:
    void ShowItemDescription(const BAK::InventoryItem& item);
    void ClearDiscounts();
    void UpdateInventoryContents();
    void AddChildren();

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
