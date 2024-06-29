#pragma once

#include "bak/types.hpp"
#include "bak/objectInfo.hpp"

namespace BAK {
class IContainer;
class FileBuffer;
class InventoryItem;
}

namespace BAK {

struct ShopStats
{
    std::uint8_t mTempleNumber;
    std::uint8_t mSellFactor;
    std::uint8_t mMaxDiscount;
    std::uint8_t mBuyFactor;
    std::uint8_t mHaggleDifficulty;
    std::uint8_t mHaggleAnnoyanceFactor;
    std::uint8_t mBardingSkill;
    std::uint8_t mBardingReward;
    std::uint8_t mBardingMaxReward;
    std::uint8_t mUnknown;
    std::uint8_t mInnSleepTilHour;
    std::uint8_t mInnCost;
    std::uint8_t mRepairTypes;
    std::uint8_t mRepairFactor;
    std::uint16_t mCategories;

    std::uint8_t GetRepairFactor() const;
    std::uint8_t GetTempleBlessFixedCost() const;
    std::uint8_t GetTempleBlessPercent() const;
    Modifier GetTempleBlessType() const;
    std::uint8_t GetTempleHealFactor() const;
};

std::ostream& operator<<(std::ostream&, const ShopStats&);

ShopStats LoadShop(FileBuffer& fb);

}

namespace BAK::Shop {

bool CanBuyItem(const InventoryItem& item, const IContainer& shop);
Royals GetSellPrice(const InventoryItem&, const ShopStats&, Royals discount, bool isRomneyGuildWars);
Royals GetBuyPrice (const InventoryItem&, const ShopStats&, bool isRomneyGuildWars);
bool CanRepair(const InventoryItem&, const ShopStats&);
Royals CalculateRepairCost(const InventoryItem&, const ShopStats&);
void RepairItem(InventoryItem& item);

double GetItemQuantityMultiple(const InventoryItem&);
}
