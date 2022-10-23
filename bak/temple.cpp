#include "bak/temple.hpp"

namespace BAK::Temple {

bool CanBlessItem(const BAK::InventoryItem& item)
{
    const auto& object = item.GetObject();
    return object.mType == ItemType::Armor
        || object.mType == ItemType::Sword;
}

bool IsBlessed(const BAK::InventoryItem& item)
{
    return item.HasModifier(Modifier::Blessing1)
        || item.HasModifier(Modifier::Blessing2)
        || item.HasModifier(Modifier::Blessing3);
}

Royals CalculateBlessPrice(const BAK::InventoryItem& item, const ShopStats& shop)
{
    const auto& object = item.GetObject();
    const auto cost = (shop.GetTempleBlessFixedCost() * 10) + ((object.mValue * shop.GetTempleBlessPercent()) / 100);
    assert(cost > 0);
    return Royals{static_cast<unsigned>(cost)};
}

void BlessItem(BAK::InventoryItem& item, const ShopStats& shop)
{
    item.SetModifier(shop.GetTempleBlessType());
}

void RemoveBlessing(BAK::InventoryItem& item)
{
    for (const auto blessType : {Modifier::Blessing1, Modifier::Blessing2, Modifier::Blessing3})
    {
        item.UnsetModifier(blessType);
    }
}

}
