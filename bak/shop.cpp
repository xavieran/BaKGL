#include "bak/shop.hpp"

#include "bak/objectInfo.hpp"
#include "bak/itemNumbers.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include <cmath>

namespace BAK {

std::ostream& operator<<(std::ostream& os, const ShopStats& shop)
{
    os << std::dec << "ShopStats { templeNumber: " << +shop.mTempleNumber
        << " sellFactor/fixedBlsCost: " << +shop.mSellFactor
        << " maxDiscount/blessPcnt: " << +shop.mMaxDiscount
        << " buyFactor/blessType: " << +shop.mBuyFactor
        << " haggle1: " << std::hex << +shop.mHaggle1 << std::dec
        << " haggle2: " << std::hex << +shop.mHaggle2 << std::dec
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

std::uint8_t ShopStats::GetRepairFactor() const
{
    return mRepairFactor;
}

std::uint8_t ShopStats::GetTempleBlessFixedCost() const
{
    return mSellFactor;
}

std::uint8_t ShopStats::GetTempleBlessPercent() const
{
    return mMaxDiscount;
}

Modifier ShopStats::GetTempleBlessType() const
{
    return static_cast<Modifier>(4 + mBuyFactor);
}

std::uint8_t ShopStats::GetTempleHealFactor() const
{
    return mHaggle1;
}

ShopStats LoadShop(FileBuffer& fb)
{
    const auto templeNumber     = fb.GetUint8();
    const auto sellFactor       = fb.GetUint8();
    const auto maxDiscount      = fb.GetUint8();
    const auto buyFactor        = fb.GetUint8();
    const auto haggle1          = fb.GetUint8();
    const auto haggle2          = fb.GetUint8();
    const auto bardingSkill     = fb.GetUint8();
    const auto bardingReward    = fb.GetUint8();
    const auto bardingMaxReward = fb.GetUint8();
    const auto unknown          = fb.GetArray<3>();
    const auto repairTypes      = fb.GetUint8();
    const auto repairFactor     = fb.GetUint8();
    const auto categories       = fb.GetUint16LE();

    return ShopStats{
        templeNumber,
        sellFactor,
        maxDiscount,
        buyFactor,
        haggle1,
        haggle2,
        bardingSkill,
        bardingReward,
        bardingMaxReward,
        unknown,
        repairTypes,
        repairFactor,
        categories
    };
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

bool CanRepair(const InventoryItem& item, const ShopStats& stats)
{
    if (item.IsItemType(ItemType::Crossbow))
    {
        return stats.mRepairTypes & 0x4;
    }
    else if (item.IsItemType(ItemType::Sword))
    {
        return stats.mRepairTypes & 0x1;
    }
    else if (item.IsItemType(ItemType::Armor))
    {
        return stats.mRepairTypes & 0x2;
    }
    else
    {
        return false;
    }
}

Royals CalculateRepairCost(const InventoryItem& item, const ShopStats& stats)
{
    if (item.IsItemType(ItemType::Crossbow))
    {
        if (item.GetItemIndex() == BAK::sBessyMauler)
            return Royals{800};
        else
            return Royals{400};
    }
    else
    {
        const auto value = item.GetObject().mValue;
        return Royals{(value * stats.GetRepairFactor() * (100 - item.GetCondition())) / 10000};
    }
}

void RepairItem(InventoryItem& item)
{
    item.SetCondition(100);
    item.SetRepairable(false);
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
