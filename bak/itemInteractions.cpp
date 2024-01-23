#include "bak/itemInteractions.hpp"

#include "bak/dialogSources.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/skills.hpp"
#include "bak/sounds.hpp"

#include "com/logger.hpp"
#include "com/random.hpp"

namespace BAK {

ItemUseResult RepairItem(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    if (!targetItem.IsItemType(static_cast<ItemType>(sourceItem.GetObject().mEffectMask)))
    {
        return ItemUseResult{
            std::nullopt,
            DialogSources::GetChoiceResult(
                DialogSources::mItemUseFailure,
                sourceItem.GetItemIndex().mValue)};
    }

    if (!targetItem.IsRepairable())
    {
        return ItemUseResult{
            std::nullopt,
            DialogSources::mCantRepairItemFurther};
    }

    const auto whichSkill = targetItem.IsItemType(ItemType::Armor)
        ? SkillType::Armorcraft
        : SkillType::Weaponcraft;
    const auto skill = character.GetSkill(whichSkill);
    character.ImproveSkill(whichSkill, SkillChange::ExercisedSkill, 1);

    const auto initialCondition = targetItem.GetCondition();
    const auto adjustment = ((100 - initialCondition) * skill) / 100;
    auto finalCondition = initialCondition + adjustment;
    if (finalCondition > 99) finalCondition = 99;

    targetItem.SetCondition(finalCondition);
    targetItem.SetRepairable(false);

    auto result = ItemUseResult{
        sourceItem.GetItemUseSound(),
        KeyTarget{0}};

    if (targetItem.IsItemType(BAK::ItemType::Crossbow))
    {
        character.RemoveItem(
            InventoryItemFactory::MakeItem(sourceItem.GetItemIndex(), 1));
    }
    else
    {
        character.GetInventory().RemoveItem(sourceItemIndex, 1);
    }

    return result;
}

ItemUseResult FixCrossbow(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    const auto isHeavyCrossbow 
        = targetItem.GetItemIndex() == sTsuraniHeavyCrossbow
        || targetItem.GetItemIndex() == sBessyMauler;
    if ((sourceItem.GetItemIndex() == sLightBowstring && !isHeavyCrossbow)
        || (sourceItem.GetItemIndex() == sHeavyBowstring && isHeavyCrossbow))
    {
        targetItem.SetBroken(false);
        targetItem.SetRepairable(false);
        targetItem.SetCondition(100);

        auto result = ItemUseResult{
            sourceItem.GetItemUseSound(),
            KeyTarget{0}};

        character.RemoveItem(sourceItem);

        return result;
    }
    else
    {
        return ItemUseResult{
            std::nullopt,
            Target{DialogSources::mGenericCantUseItem}};
    }
}

ItemUseResult ModifyItem(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    if (!targetItem.IsModifiableBy(sourceItem.GetObject().mType))
    {
        return ItemUseResult{
            std::nullopt,
            DialogSources::GetChoiceResult(
                DialogSources::mItemUseFailure,
                sourceItem.GetItemIndex().mValue)};
    }

    targetItem.ClearTemporaryModifiers();
    targetItem.SetStatusAndModifierFromMask(sourceItem.GetObject().mEffectMask);

    auto result = ItemUseResult{
        sourceItem.GetItemUseSound(),
        DialogSources::GetChoiceResult(
            DialogSources::mItemUseSucessful,
            sourceItem.GetItemIndex().mValue)};

    if (sourceItem.IsChargeBased() || sourceItem.IsQuantityBased())
        character.RemoveItem(
            InventoryItemFactory::MakeItem(sourceItem.GetItemIndex(), 1));

    return result;
}

ItemUseResult PoisonQuarrel(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    Logging::LogDebug(__FUNCTION__) << " Poison Quarrel " << targetItem << " with " << sourceItem << "\n";

    const auto newQuarrels = InventoryItemFactory::MakeItem(
        ItemIndex{targetItem.GetItemIndex().mValue + sPoisonOffset},
        targetItem.GetQuantity());

    auto result = ItemUseResult{
        sourceItem.GetItemUseSound(),
        KeyTarget{0}};

    if (sourceItem.IsChargeBased() || sourceItem.IsQuantityBased())
        character.RemoveItem(
            InventoryItemFactory::MakeItem(sourceItem.GetItemIndex(), 1));

    character.RemoveItem(targetItem);
    character.GiveItem(newQuarrels);

    return result;
}

bool IsPoisonableQuarrel(const InventoryItem& item)
{
    const auto index = item.GetItemIndex().mValue;
    return index >= sQuarrels.mValue && index <= sTsuraniQuarrels.mValue;
}

ItemUseResult PoisonRations(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    Logging::LogDebug(__FUNCTION__) << " Poison Rations " << targetItem << " with " << sourceItem << "\n";

    const auto newRations = InventoryItemFactory::MakeItem(
        sPoisonedRations,
        targetItem.GetQuantity());

    auto result = ItemUseResult{
        sourceItem.GetItemUseSound(),
        KeyTarget{0}};

    if (sourceItem.IsChargeBased() || sourceItem.IsQuantityBased())
        character.RemoveItem(
            InventoryItemFactory::MakeItem(sourceItem.GetItemIndex(), 1));

    character.RemoveItem(targetItem);
    character.GiveItem(newRations);

    return result;
}

ItemUseResult MakeGuardaRevanche(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    Logging::LogDebug(__FUNCTION__) << " Make Guarda Revanche " << targetItem << " with " << sourceItem << "\n";

    const auto guardaRevanche = InventoryItemFactory::MakeItem(
        sGuardaRevanche, 100);

    auto result = ItemUseResult{
        // Maybe a different sound?
        std::make_pair(sTeleportSound, 0),
        DialogSources::GetChoiceResult(
            DialogSources::mItemUseSucessful,
            sourceItem.GetItemIndex().mValue)};

    character.RemoveItem(sourceItem);
    character.RemoveItem(targetItem);
    character.GiveItem(guardaRevanche);

    return result;
}

ItemUseResult ApplyItemTo(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    if (sourceItem.IsRepairItem())
    {
        Logging::LogDebug(__FUNCTION__) << " Repair " << targetItem << " with " << sourceItem << "\n";
        return RepairItem(character, sourceItemIndex, targetItemIndex);
    }
    else if (sourceItem.IsItemModifier()
        && sourceItem.GetItemIndex() != sColtariPoison
        && !IsPoisonableQuarrel(targetItem))
    {
        // If incompatible item, dialog mItemUseFailure for modifiers
        Logging::LogDebug(__FUNCTION__) << " Modify item " << targetItem << " with " << sourceItem << "\n";
        return ModifyItem(character, sourceItemIndex, targetItemIndex);
    }
    else if ((sourceItem.IsItemModifier() && sourceItem.GetObject().mEffectMask & 0x80)
        && IsPoisonableQuarrel(targetItem))
    {
        Logging::LogDebug(__FUNCTION__) << " Poison quarrel " << targetItem << " with " << sourceItem << "\n";
        return PoisonQuarrel(character, sourceItemIndex, targetItemIndex);
    }
    else if (sourceItem.IsItemType(ItemType::Bowstring)
        && targetItem.IsItemType(ItemType::Crossbow))
    {
        Logging::LogDebug(__FUNCTION__) << " Fix Crossbow " << targetItem << " with " << sourceItem << "\n";
        return FixCrossbow(character, sourceItemIndex, targetItemIndex);
    }
    else if (sourceItem.GetItemIndex() == sColtariPoison
        && targetItem.GetItemIndex() == sRations)
    {
        Logging::LogDebug(__FUNCTION__) << " Poison rations " << targetItem << " with " << sourceItem << "\n";
        return PoisonRations(character, sourceItemIndex, targetItemIndex);
    }
    else if (sourceItem.GetItemIndex() == sEliaemsShell 
        && targetItem.GetItemIndex() == sExoticSword)
    {
        Logging::LogDebug(__FUNCTION__) << " Make Guarda Revanche" << targetItem << " with " << sourceItem << "\n";
        return MakeGuardaRevanche(character, sourceItemIndex, targetItemIndex);
    }
    else
    {
        Logging::LogDebug(__FUNCTION__) << " DO NOTHING " << targetItem << " with " << sourceItem << "\n";
    }

    return ItemUseResult{
        std::nullopt,
        KeyTarget{0}};
}

/*
ItemUseResult ReadBook(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    auto& object = item.GetObject();
    assert(item.GetCondition() > 0);
    assert(object.mEffectMask != 0);

    bool haveReadBefore = false;
    std::optional<std::pair<SkillChange, unsigned>> improvement;
    if (haveReadBefore)
    {
        auto chance = GetRandomNumber(0, 0xfff) % 0x64;
        if (chance > object.mPotionPowerOrBookChance)
        {
            improvement = std::make_pair(SkillChange::DifferenceOfSkill, object.mAlternativeEffect);
        }
    }
    else
    {
        improvement = std::make_pair(SkillChange::Direct, object.mEffect);
    }
    
    if (improvement)
    {
        auto [skillChange, amount] = *improvement;
        for (unsigned i = 0; i < 0x10; i++)
        {
            auto skill = static_cast<SkillType>(i);
            if (CheckBitSet(object.mEffectMask, skill))
            {
                character.ImproveSkill(skill, skillChange, amount);
            }
        }
    }
}

ItemUseResult UseItem(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    auto& object = item.GetObject();
    if  (object.mType == ItemType::Book)
    {
        ReadBook(character, inventoryIndex);
    }
}
*/
}
