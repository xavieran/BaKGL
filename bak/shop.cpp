#include "bak/shop.hpp"

namespace BAK::Shop {

Royals GetSellPrice(const BAK::InventoryItem& item, const ShopStats& stats)
{
    // FIXME: Add blessing value change
    // Add Scroll exception
    // Add Armor exception
    const auto sellFactor = static_cast<double>(100 + stats.mSellFactor) / 100.0;
    const auto baseValue = static_cast<double>(item.GetObject().mValue);
    const auto itemQuantity = GetItemQuantityMultiple(item);
    return Royals{static_cast<unsigned>(sellFactor * baseValue * itemQuantity)};
}

Royals GetBuyPrice (const BAK::InventoryItem& item, const ShopStats& stats)
{
    const auto buyFactor = static_cast<double>(stats.mBuyFactor) / 100.0;
    return Royals{static_cast<unsigned>(buyFactor 
        * GetSellPrice(item, stats).mValue)};
}

double GetItemQuantityMultiple(const BAK::InventoryItem& item)
{
    if (item.IsStackable() || item.IsChargeBased())
    {
        return static_cast<double>(item.GetQuantity()) 
            / static_cast<double>(item.GetObject().mDefaultStackSize);
    }
    else if (item.IsConditionBased())
    {
        return static_cast<double>(item.GetCondition()) / 100.0;
    }
    else
    {
        return 1.0;
    }
}

}
