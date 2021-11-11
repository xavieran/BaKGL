#include "bak/shop.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include <cmath>

namespace BAK {

std::ostream& operator<<(std::ostream& os, const ShopStats& shop)
{
    os << std::dec << "ShopStats { templeNumber: " << +shop.mTempleNumber
        << " sellFactor: " << +shop.mSellFactor
        << " maxDiscount: " << +shop.mMaxDiscount
        << " buyFactor: " << +shop.mBuyFactor
        << " haggle: " << std::hex << shop.mHaggle << std::dec
        << " bardingSkill: " << +shop.mBardingSkill
        << " bardingReward: " << +shop.mBardingReward
        << " bardingMaxReward: " << +shop.mBardingMaxReward
        << " unknown: " << std::hex << shop.mUnknown << std::dec
        << " repairTypes: " << +shop.mRepairTypes
        << " repairFactor: " << +shop.mRepairFactor
        << " categories: " << std::hex << shop.mCategories << std::dec
        << " [" << GetCategories(shop.mCategories) << "]}";

    return os;
}

}

namespace BAK::Shop {

Royals GetSellPrice(const BAK::InventoryItem& item, const ShopStats& stats)
{
    // FIXME: Add blessing value change
    // Add Scroll exception
    // Add Armor exception
    const auto sellFactor = static_cast<double>(100 + stats.mSellFactor) / 100.0;
    const auto baseValue = static_cast<double>(item.GetObject().mValue);
    const auto itemQuantity = GetItemQuantityMultiple(item);
    const auto value = sellFactor * baseValue * itemQuantity;
    const auto money = Royals{static_cast<unsigned>(std::round(value))};
    return money;
}

Royals GetBuyPrice (const BAK::InventoryItem& item, const ShopStats& stats)
{
    const auto buyFactor = static_cast<double>(stats.mBuyFactor) / 100.0;
    const auto sellPrice = static_cast<double>(GetSellPrice(item, stats).mValue);
    const auto price = Royals{static_cast<unsigned>(std::round(buyFactor * sellPrice))};
    return price;
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
