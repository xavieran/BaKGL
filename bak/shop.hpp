#pragma once

#include "bak/money.hpp"
#include "bak/objectInfo.hpp"
#include "bak/IContainer.hpp"
#include "bak/inventoryItem.hpp"

#include "bak/fileBuffer.hpp"

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
    std::array<std::uint8_t, 3> mUnknown;
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

bool CanBuyItem(const BAK::InventoryItem& item, const BAK::IContainer& shop);
Royals GetSellPrice(const InventoryItem&, const ShopStats&, Royals discount);
Royals GetBuyPrice (const InventoryItem&, const ShopStats&);
bool CanRepair(const InventoryItem&, const ShopStats&);
Royals CalculateRepairCost(const InventoryItem&, const ShopStats&);
void RepairItem(InventoryItem& item);

double GetItemQuantityMultiple(const InventoryItem&);
}
