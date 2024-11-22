#include "bak/state/customStateChoice.hpp"

#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/coordinates.hpp"

namespace BAK::State {

CustomStateEvaluator::CustomStateEvaluator(const GameState& gameState)
:
    mGameState{gameState}
{}

bool CustomStateEvaluator::AnyCharacterStarving() const
{
    bool foundStarving = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            if (character.GetConditions().GetCondition(Condition::Starving).Get() > 0)
            {
                foundStarving = true;
                return Loop::Finish;
            }
            return Loop::Continue;
        });

    return foundStarving;
}

bool CustomStateEvaluator::Plagued() const
{
    bool foundPlagued = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            if (character.GetConditions().GetCondition(Condition::Plagued).Get() > 0)
            {
                foundPlagued = true;
                return Loop::Finish;
            }
            return Loop::Continue;
        });

    return foundPlagued;
}

bool CustomStateEvaluator::HaveSixSuitsOfArmor() const
{
    unsigned armorCount = 0;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            const auto& items = character.GetInventory().GetItems();
            for (const auto& item : items)
            {
                if (item.GetItemIndex() == sStandardArmor)
                {
                    armorCount++;
                }
            }
            return Loop::Continue;
        });
    return armorCount >= 6;
}

bool CustomStateEvaluator::AllPartyArmorIsGoodCondition() const
{
    bool foundRepairableArmor = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            const auto& items = character.GetInventory().GetItems();
            for (const auto& item : items)
            {
                if (item.IsItemType(ItemType::Armor) && item.IsRepairableByShop())
                {
                    foundRepairableArmor = true;
                    return Loop::Finish;
                }
            }
            return Loop::Continue;
        });

    return !foundRepairableArmor;
}

bool CustomStateEvaluator::PoisonedDelekhanArmyChests() const
{
    unsigned foundPoisonedRations = 0;
    for (const auto& [zone, x, y] : {
        std::make_tuple(5u, 0x16b2fb, 0x111547),
        std::make_tuple(5u, 0x16b2fb, 0x110f20),
        std::make_tuple(5u, 0x16b33a, 0x11083c)})
    {
        auto* container = mGameState.GetWorldContainer(ZoneNumber{zone}, GamePosition{x, y});
        ASSERT(container);
        if (container->GetInventory().HaveItem(InventoryItemFactory::MakeItem(sPoisonedRations, 1)))
        {
            foundPoisonedRations++;
        }
    }
    return foundPoisonedRations == 3;
}

bool CustomStateEvaluator::AnyCharacterSansWeapon() const
{
    bool noWeapon = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            if (character.HasEmptyStaffSlot() || character.HasEmptySwordSlot())
            {
                noWeapon = true;
                return Loop::Finish;
            }
            return Loop::Continue;
        });

    return !noWeapon;
}

bool CustomStateEvaluator::AnyCharacterHasNegativeCondition() const
{
    bool nonZero = false;
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        for (unsigned i = 0; i < 7; i++)
        {
            if (i == 4) continue;
            if (character.GetConditions().GetCondition(static_cast<Condition>(i)).Get() > 0)
            {
                nonZero = true;
                return Loop::Finish;
            }
        }
        return Loop::Continue;
    });
    return nonZero;
}

bool CustomStateEvaluator::AnyCharacterIsUnhealthy() const
{
    if (AnyCharacterHasNegativeCondition())
    {
        return true;
    }

    bool nonZero = false;
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        if (character.GetSkill(SkillType::TotalHealth) != character.GetMaxSkill(SkillType::TotalHealth))
        {
            nonZero = true;
            return Loop::Finish;
        }
        return Loop::Continue;
    });
    return nonZero;
}

bool CustomStateEvaluator::AllCharactersHaveNapthaMask() const
{
    bool noMask = false;
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        if (!character.GetInventory().HaveItem(
            InventoryItemFactory::MakeItem(sNapthaMask, 1)))
        {
            noMask = true;
            return Loop::Finish;
        }
        return Loop::Continue;
    });
    return noMask;
}

bool CustomStateEvaluator::NormalFoodInArlieChest() const
{
    auto* container = mGameState.GetWorldContainer(ZoneNumber{3}, GamePosition{1308000, 1002400});
    ASSERT(container);
    return container->GetInventory().HaveItem(InventoryItemFactory::MakeItem(sRations, 1));
}

bool CustomStateEvaluator::PoisonedFoodInArlieChest() const
{
    auto* container = mGameState.GetWorldContainer(ZoneNumber{3}, GamePosition{1308000, 1002400});
    ASSERT(container);
    return container->GetInventory().HaveItem(InventoryItemFactory::MakeItem(sPoisonedRations, 1));
}

}
