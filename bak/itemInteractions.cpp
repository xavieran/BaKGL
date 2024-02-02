#include "bak/itemInteractions.hpp"

#include "bak/bard.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/skills.hpp"
#include "bak/sounds.hpp"
#include "bak/state/item.hpp"

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
            sourceItem.GetItemIndex().mValue,
            DialogSources::mItemUseFailure};
    }

    if (!targetItem.IsRepairable())
    {
        return ItemUseResult{
            std::nullopt,
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
        std::nullopt,
        KeyTarget{0}};

    if (targetItem.IsItemType(BAK::ItemType::Crossbow))
    {
        character.GetInventory().RemoveItem(sourceItemIndex, 1);
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
            std::nullopt,
            KeyTarget{0}};

        character.GetInventory().RemoveItem(sourceItemIndex, 1);

        return result;
    }
    else
    {
        return ItemUseResult{
            std::nullopt,
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
            sourceItem.GetItemIndex().mValue,
            DialogSources::mItemUseFailure};
    }

    targetItem.ClearTemporaryModifiers();
    targetItem.SetStatusAndModifierFromMask(sourceItem.GetObject().mEffectMask);

    auto result = ItemUseResult{
        sourceItem.GetItemUseSound(),
        sourceItem.GetItemIndex().mValue,
        DialogSources::mItemUseSucessful};

    if (sourceItem.IsChargeBased() || sourceItem.IsQuantityBased())
    {
        character.GetInventory().RemoveItem(sourceItemIndex, 1);
    }

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
        std::nullopt,
        KeyTarget{0}};

    if (sourceItem.IsChargeBased() || sourceItem.IsQuantityBased())
    {
        character.GetInventory().RemoveItem(sourceItemIndex, 1);
    }

    character.GetInventory().ReplaceItem(targetItemIndex, newQuarrels);

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
        std::nullopt,
        KeyTarget{0}};

    if (sourceItem.IsChargeBased() || sourceItem.IsQuantityBased())
    {
        character.GetInventory().RemoveItem(sourceItemIndex, 1);
    }

    character.GetInventory().ReplaceItem(targetItemIndex, newRations);

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
        sourceItem.GetItemIndex().mValue,
        DialogSources::mItemUseSucessful};

    character.GetInventory().RemoveItem(sourceItemIndex);
    character.GetInventory().RemoveItem(targetItemIndex);
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
        std::nullopt,
        KeyTarget{0}};
}

ItemUseResult ReadBook(
    GameState& gameState,
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    if (item.GetQuantity() == 0)
    {
        return ItemUseResult{
            std::nullopt,
            std::nullopt,
            KeyTarget{DialogSources::mItemHasNoCharges}
        };
    }

    auto& object = item.GetObject();
    assert(object.mEffectMask != 0);

    bool haveReadBefore = gameState.Apply(
        State::ReadItemHasBeenUsed,
        character.mCharacterIndex.mValue,
        item.GetItemIndex().mValue);
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
        gameState.Apply(
            State::SetItemHasBeenUsed,
            character.mCharacterIndex.mValue,
            item.GetItemIndex().mValue);
        improvement = std::make_pair(SkillChange::Direct, static_cast<unsigned>(object.mEffect) << 8);
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

    item.SetQuantity(item.GetQuantity() - 1);

    return ItemUseResult{
        std::nullopt,
        item.GetItemIndex().mValue,
        DialogSources::mItemUseSucessful};
}

ItemUseResult LearnSpell(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    assert(character.IsSpellcaster() && item.IsMagicUserOnly());

    if (character.GetSpells().HaveSpell(item.GetSpell()))
    {
        return ItemUseResult{
            std::nullopt,
            item.GetItemIndex().mValue,
            BAK::DialogSources::mItemUseFailure
        };
    }
    else
    {
        auto sound = item.GetItemUseSound();
        auto itemIndex = item.GetItemIndex();
        character.GetSpells().SetSpell(item.GetSpell());
        character.GetInventory().RemoveItem(inventoryIndex);
        return ItemUseResult{
            sound,
            itemIndex.mValue,
            BAK::DialogSources::mItemUseSucessful
        };
    }
}

ItemUseResult PractiseBarding(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    auto bardSkill = character.GetSkill(SkillType::Barding);
    const auto ClassifyBardSkill = [](auto bardSkill)
    {
        if (bardSkill < 80)
        {
            if (bardSkill < 65)
            {
                if (bardSkill < 45)
                {
                    return Bard::BardStatus::Failed;
                }
                else
                {
                    return Bard::BardStatus::Poor;
                }
            }
            else
            {
                return Bard::BardStatus::Good;
            }
        }
        else
        {
            return Bard::BardStatus::Best;
        }
    };

    unsigned bardSong = Bard::GetSong(ClassifyBardSkill(bardSkill));
    
    auto improvement = 0x28 + (GetRandomNumber(0, 0xfff) % 120);
    character.ImproveSkill(SkillType::Barding, SkillChange::Direct, improvement);
    auto itemIndex = item.GetItemIndex().mValue;
    character.GetInventory().RemoveItem(inventoryIndex, 1);
    return ItemUseResult{
        //std::make_pair(bardSong, 1),
        std::nullopt,
        itemIndex,
        DialogSources::mItemUseSucessful
    };
}

ItemUseResult UseConditionModifier(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    if (item.GetQuantity() == 0)
    {
        return ItemUseResult{
            std::nullopt,
            1,
            KeyTarget{DialogSources::mItemHasNoCharges}
        };
    }

    character.GetConditions().AdjustCondition(
        character.GetSkills(),
        static_cast<BAK::Condition>(item.GetObject().mEffectMask),
        item.GetObject().mEffect);

    auto itemIndex = item.GetItemIndex().mValue;
    if (item.IsConsumable())
    {
        character.GetInventory().RemoveItem(inventoryIndex, 1);
    }
    else
    {
        item.SetQuantity(item.GetQuantity() - 1);
    }

    return ItemUseResult{
        item.GetItemUseSound(),
        itemIndex,
        DialogSources::mItemUseSucessful
    };
}

ItemUseResult CurePoisoned(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    if (character.GetConditions().GetCondition(BAK::Condition::Poisoned).Get() == 0)
    {
        return ItemUseResult{
            item.GetItemUseSound(),
            item.GetItemIndex().mValue,
            DialogSources::mItemUseFailure
        };
    }

    character.GetConditions().AdjustCondition(
        character.GetSkills(),
        BAK::Condition::Poisoned,
        -100);

    auto itemIndex = item.GetItemIndex().mValue;
    character.GetInventory().RemoveItem(inventoryIndex, 1);

    return ItemUseResult{
        item.GetItemUseSound(),
        std::nullopt,
        DialogSources::mConsumeAntiVenom
    };
}

ItemUseResult UseRestoratives(
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);

    const int adjustAmount = -5;
    for (unsigned i = 0; i < 7; i++)
    {
        auto cond = static_cast<BAK::Condition>(i);
        if (cond == BAK::Condition::Healing)
        {
            continue;
        }

        character.GetConditions().AdjustCondition(
            character.GetSkills(),
            cond,
            adjustAmount);
    }

    const auto healthRecoverAmount = item.GetObject().mEffectMask + GetRandomNumber(0, 0xfff) % 2;
    character.GetSkills().ImproveSkill(
        character.GetConditions(),
        SkillType::TotalHealth,
        SkillChange::HealMultiplier_100,
        healthRecoverAmount << 8);

    auto result = ItemUseResult{
        item.GetItemUseSound(),
        item.GetItemIndex().mValue,
        DialogSources::mItemUseSucessful};

    character.GetInventory().RemoveItem(inventoryIndex, 1);

    return result;
}

ItemUseResult UsePotion(
    GameState& gameState,
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);

    auto skill = ToSkill(BAK::SkillTypeMask{item.GetObject().mEffect});
    auto adjust = item.GetObject().mPotionPowerOrBookChance;
    auto startTime = gameState.GetWorldTime().GetTime();
    auto endTime = startTime + (Times::OneHour * item.GetObject().mAlternativeEffect);

    bool alreadyUsedThisPotionRecently{false};
    for (const auto& affector : character.GetSkillAffectors())
    {
        Logging::LogDebug(__FUNCTION__) << " TargetSkill: " << ToString(skill) << " Affector: " << affector <<
            " time: " << startTime << "\n";
        if (affector.mSkill == skill
            && affector.mEndTime >= startTime)
        {
            alreadyUsedThisPotionRecently = true;
            break;
        }
    }

    if (alreadyUsedThisPotionRecently)
    {
        return ItemUseResult{
            std::nullopt,
            item.GetItemIndex().mValue,
            DialogSources::mCantConsumeMorePotion};
    }

    character.AddSkillAffector(SkillAffector{0x200, skill, adjust, startTime, endTime});

    auto result = ItemUseResult{
        item.GetItemUseSound(),
        item.GetItemIndex().mValue,
        DialogSources::mItemUseSucessful};

    character.GetInventory().RemoveItem(inventoryIndex, 1);

    return result;
}

ItemUseResult UseItem(
    GameState& gameState,
    Character& character,
    InventoryIndex inventoryIndex)
{
    auto& item = character.GetInventory().GetAtIndex(inventoryIndex);
    auto& object = item.GetObject();

    if (item.IsMagicUserOnly() && !character.IsSpellcaster())
    {
        return ItemUseResult{
            std::nullopt,
            item.GetItemIndex().mValue,
            BAK::DialogSources::mWarriorCantUseMagiciansItem};
    }
    else if (item.IsSwordsmanUserOnly() && !character.IsSwordsman())
    {
        return ItemUseResult{
            std::nullopt,
            item.GetItemIndex().mValue,
            BAK::DialogSources::mMagicianCantUseWarriorsItem};

    }

    if  (object.mType == ItemType::Book)
    {
        return ReadBook(gameState, character, inventoryIndex);
    }
    else if (item.IsItemType(BAK::ItemType::Note))
    {
        if (item.GetItemIndex() == sSpynote)
        {
            return ItemUseResult{
                std::nullopt,
                item.GetQuantity(),
                BAK::DialogSources::GetSpynote()};
        }
        else
        {
            return ItemUseResult{
                std::nullopt,
                item.GetItemIndex().mValue,
                DialogSources::mItemUseSucessful};
        }
    }
    else if (item.IsItemType(BAK::ItemType::Scroll))
    {
        return LearnSpell(character, inventoryIndex);
    }
    else if (item.GetItemIndex() == sPractiseLute)
    {
        return PractiseBarding(character, inventoryIndex);
    }
    else if (item.IsItemType(BAK::ItemType::ConditionModifier))
    {
        return UseConditionModifier(character, inventoryIndex);
    }
    else if (item.IsItemType(BAK::ItemType::Restoratives))
    {
        return UseRestoratives(character, inventoryIndex);
    }
    else if (item.IsItemType(BAK::ItemType::Potion))
    {
        return UsePotion(gameState, character, inventoryIndex);
    }
    else if (item.GetItemIndex() == sSilverthornAntiVenom)
    {
        return CurePoisoned(character, inventoryIndex);
    }

    return ItemUseResult{
        std::nullopt,
        std::nullopt,
        KeyTarget{0}};
}
}
