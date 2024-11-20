#include "bak/shop.hpp"

#include "bak/IContainer.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/money.hpp"

#include "bak/objectInfo.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include <algorithm>
#include <cmath>

namespace BAK {

std::ostream& operator<<(std::ostream& os, const ShopStats& shop)
{
    os << std::dec << "ShopStats { templeNumber: " << +shop.mTempleNumber
        << " sellFactor/fixedBlsCost: " << +shop.mSellFactor
        << " maxDiscount/blessPcnt: " << +shop.mMaxDiscount
        << " buyFactor/blessType: " << +shop.mBuyFactor
        << " haggleDifficulty: " << std::hex << +shop.mHaggleDifficulty << std::dec
        << " haggleAnnoyanceFactor: " << std::hex << +shop.mHaggleAnnoyanceFactor << std::dec
        << " bardingSkill: " << +shop.mBardingSkill
        << " bardingReward: " << +shop.mBardingReward
        << " bardingMaxReward: " << +shop.mBardingMaxReward
        << " unknown: " << std::hex << shop.mUnknown << std::dec
        << " innSleepTilHour: " << +shop.mInnSleepTilHour
        << " innCost: " << +shop.mInnCost
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
    return mHaggleDifficulty;
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
    const auto unknown          = fb.GetUint8();
    const auto innSleepTilHour  = fb.GetUint8();
    const auto innCost          = fb.GetUint8();
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
        innSleepTilHour,
        innCost,
        repairTypes,
        repairFactor,
        categories
    };
}

}

namespace BAK::Shop {

Royals GetSellPrice(const BAK::InventoryItem& item, const ShopStats& stats, Royals discount, bool isRomneyGuildWars)
{
    const auto sellFactor = static_cast<double>(100 + stats.mSellFactor) / 100.0;
    if (item.GetObject().mType == ItemType::Scroll)
    {
        return ObjectIndex::Get().GetScrollValue(item.GetSpell());
    }

    auto baseValue = static_cast<double>(item.GetObject().mValue);

    for (auto mod : item.GetModifiers())
    {
        switch (mod)
        {
            case Modifier::Blessing1:
                baseValue = (baseValue * 150) / 100;
                break;
            case Modifier::Blessing2:
                baseValue = (baseValue * 175) / 100;
                break;
            case Modifier::Blessing3:
                baseValue = (baseValue * 200) / 100;
                break;
            default:
                break;
        }
    }

    if (baseValue <= 0) baseValue = 1;

    auto shopBasicValue = std::clamp(
        sellFactor * baseValue - discount.mValue,
        1.0, sellFactor * baseValue);

    if (isRomneyGuildWars)
    {
        shopBasicValue = (shopBasicValue * 600) / 100;
    }

    const auto value = shopBasicValue * GetItemQuantityMultiple(item);
    const auto money = Royals{static_cast<unsigned>(std::round(value))};
    return money;
}

Royals GetBuyPrice (const BAK::InventoryItem& item, const ShopStats& stats, bool isRomneyGuildWars)
{
    const auto buyFactor = static_cast<double>(stats.mBuyFactor) / 100.0;
    const auto sellPrice = static_cast<double>(GetSellPrice(item, stats, Royals{0}, isRomneyGuildWars).mValue);
    auto buyPrice = static_cast<unsigned>(std::round(buyFactor * sellPrice));
    if (item.IsItemType(BAK::ItemType::Armor))
    {
        buyPrice >>= 1;
    }
    return Royals{buyPrice};
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
    item.SetBroken(false);
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

bool CanBuyItem(const BAK::InventoryItem& item, const BAK::IContainer& shop)
{
    return shop.GetInventory().HaveItem(item) ||
        (item.GetObject().mCategories & shop.GetShop().mCategories) != 0x0000;
}

}
