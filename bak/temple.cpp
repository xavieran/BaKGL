#include "bak/temple.hpp"

#include "bak/condition.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/skills.hpp"
#include "bak/shop.hpp"

namespace BAK::Temple {

bool CanBlessItem(const InventoryItem& item)
{
    const auto& object = item.GetObject();
    return object.mType == ItemType::Armor
        || object.mType == ItemType::Sword;
}

bool IsBlessed(const InventoryItem& item)
{
    return item.HasModifier(Modifier::Blessing1)
        || item.HasModifier(Modifier::Blessing2)
        || item.HasModifier(Modifier::Blessing3);
}

Royals CalculateBlessPrice(const InventoryItem& item, const ShopStats& shop)
{
    const auto& object = item.GetObject();
    const auto cost = (shop.GetTempleBlessFixedCost() * 10) + ((object.mValue * shop.GetTempleBlessPercent()) / 100);
    assert(cost > 0);
    return Royals{static_cast<unsigned>(cost)};
}

void BlessItem(InventoryItem& item, const ShopStats& shop)
{
    item.SetModifier(shop.GetTempleBlessType());
}

void RemoveBlessing(InventoryItem& item)
{
    for (const auto blessType : {Modifier::Blessing1, Modifier::Blessing2, Modifier::Blessing3})
    {
        item.UnsetModifier(blessType);
    }
}

Royals CalculateTeleportCost(
    unsigned srcTemple,
    unsigned dstTemple,
    glm::vec2 srcPos,
    glm::vec2 dstPos,
    unsigned teleportMultiplier,
    unsigned teleportConstant)
{
    ASSERT(srcTemple > 0 && dstTemple > 0);
    ASSERT(srcTemple <= 13 && dstTemple <= 13);
    auto xDiff = static_cast<unsigned>(std::abs(srcPos.x - dstPos.x));
    auto yDiff = static_cast<unsigned>(std::abs(srcPos.y - dstPos.y));
    auto cost = std::max(xDiff, yDiff) + (std::min(xDiff, yDiff) * 3) / 8;
    cost = ((cost * teleportMultiplier) + teleportConstant) * 10;
    cost = (cost + 5) / 10;
    return Royals{cost};
}

Royals CalculateCureCost(
    unsigned cureFactor,
    bool isTempleOfSung,
    Skills& skills,
    const Conditions& conditions,
    const std::vector<SkillAffector>& skillAffectors)
{
    static constexpr unsigned conditionCost[7] = {
        4, 10, 10, 3, 0, 2, 30
    };

    unsigned totalCost = 0;
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = conditions.GetCondition(static_cast<Condition>(i));
        if (i != static_cast<unsigned>(Condition::Healing) && cond.Get() > 0)
            totalCost += (cond.Get() * conditionCost[i]) + 10;
    }

    totalCost = (totalCost * cureFactor) / 100;

    if (isTempleOfSung)
    {
        totalCost += CalculateEffectiveSkillValue(
            SkillType::TotalHealth,
            skills,
            conditions,
            skillAffectors,
            SkillRead::MaxSkill);
        totalCost -= CalculateEffectiveSkillValue(
            SkillType::TotalHealth,
            skills,
            conditions,
            skillAffectors,
            SkillRead::Current);
    }

    return Royals{totalCost};
}

void CureCharacter(Skills& skills, Conditions& conditions, bool isTempleOfSung)
{
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = static_cast<Condition>(i);
        const auto amount = cond == Condition::Healing ? 20 : -100;
        conditions.AdjustCondition(skills, cond, amount);
    }
    
    if (isTempleOfSung)
    {
        DoAdjustHealth(skills, conditions, 100, 0x7fff);
        conditions.AdjustCondition(skills, Condition::Healing, 100);
    }
}

}
