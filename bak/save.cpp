#include "bak/save.hpp"

#include "bak/gameData.hpp"

namespace BAK {

void Save(const Inventory& inv, FileBuffer& fb)
{
    const auto itemCount = inv.GetNumberItems();
    const auto capacity = inv.GetCapacity();

    unsigned i = 0;
    for (; i < itemCount; i++)
    {
        const auto& item = inv.GetAtIndex(InventoryIndex{i});
        fb.PutUint8(item.GetItemIndex().mValue);
        fb.PutUint8(item.GetCondition());
        fb.PutUint8(item.GetStatus());
        fb.PutUint8(item.GetModifierMask());
    }

    for (; i < capacity; i++)
        fb.Skip(4);
}

void Save(const GenericContainer& gc, FileBuffer& fb)
{
    // Write container header
    fb.Seek(gc.GetHeader().GetAddress());
    fb.Skip(12); // Skip mLocation, that won't change
    fb.Skip(1); // Skip mLocationType
    fb.PutUint8(gc.GetInventory().GetNumberItems());
    fb.Skip(1);
    fb.Skip(1);

    Save(gc.GetInventory(), fb);

    if (gc.HasLock())
    {
        const auto& lock = gc.GetLock();
        fb.PutUint8(lock.mLockFlag);
        fb.PutUint8(lock.mRating);
        fb.PutUint8(lock.mFairyChestIndex);
        fb.PutUint8(lock.mTrapDamage);
    }

    if (gc.HasDialog())
        fb.Skip(6); // Don't need to write anything for dialog

    if (gc.HasShop())
    {
        const auto& shop = gc.GetShop();
        fb.PutUint8(shop.mTempleNumber);
        fb.PutUint8(shop.mSellFactor);
        fb.PutUint8(shop.mMaxDiscount);
        fb.PutUint8(shop.mBuyFactor);
        fb.PutUint8(shop.mHaggleDifficulty);
        fb.PutUint8(shop.mHaggleAnnoyanceFactor);
        fb.PutUint8(shop.mBardingSkill);
        fb.PutUint8(shop.mBardingReward);
        fb.PutUint8(shop.mBardingMaxReward);
        fb.PutUint8(shop.mUnknown);
        fb.PutUint8(shop.mInnSleepTilHour);
        fb.PutUint8(shop.mInnCost);
        fb.PutUint8(shop.mRepairTypes);
        fb.PutUint8(shop.mRepairFactor);
        fb.PutUint16LE(shop.mCategories);
    }

    if (gc.HasEncounter())
        fb.Skip(9); // Don't need to write anything for encounter

    if (gc.HasLastAccessed())
        fb.PutUint32LE(gc.GetLastAccessed().mTime);
}

void Save(const Character& c, FileBuffer& fb)
{
    const auto charIndex = c.mCharacterIndex.mValue;
    // Skills
    fb.Seek(BAK::GameData::GetCharacterSkillOffset(charIndex));
    fb.Skip(2); // Character name offset
    auto* spells = reinterpret_cast<const std::uint8_t*>(&c.GetSpells().GetSpellBytes());
    for (unsigned i = 0; i < 6; i++)
    {
        fb.PutUint8(spells[i]);
    }

    const auto& skills = c.GetSkills();
    for (unsigned i = 0; i < Skills::sSkills; i++)
    {
        const auto& skill  = skills.GetSkill(static_cast<BAK::SkillType>(i));
        fb.PutUint8(skill.mMax);
        fb.PutUint8(skill.mTrueSkill);
        fb.PutUint8(skill.mCurrent);
        fb.PutUint8(skill.mExperience);
        fb.PutUint8(skill.mModifier);

        const auto pos = fb.Tell();
        // FIXME: set skill selected, set skill unseen improvement...
    }

    // Inventory
    fb.Seek(BAK::GameData::GetCharacterInventoryOffset(charIndex));
    fb.PutUint8(c.GetInventory().GetNumberItems());
    fb.PutUint16LE(c.GetInventory().GetCapacity());
    Save(c.GetInventory(), fb);

    // Conditions
    fb.Seek(BAK::GameData::GetCharacterConditionOffset(charIndex));
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = c.mConditions.GetCondition(static_cast<BAK::Condition>(i));
        fb.PutUint8(cond.Get());
    }

    fb.Seek(BAK::GameData::GetCharacterAffectorsOffset(charIndex));
    for (const auto& affector : c.GetSkillAffectors())
    {
        fb.PutUint16LE(affector.mType);
        fb.PutUint16LE(1 << static_cast<std::uint16_t>(affector.mSkill));
        fb.PutSint16LE(affector.mAdjustment);
        fb.PutUint32LE(affector.mStartTime.mTime);
        fb.PutUint32LE(affector.mEndTime.mTime);
    }
}

void Save(const Party& party, FileBuffer& fb)
{
    fb.Seek(GameData::sGoldOffset);
    fb.PutUint32LE(party.GetGold().mValue);

    for (const auto& character : party.mCharacters)
    {
        Save(character, fb);
    }

    fb.Seek(BAK::GameData::sPartyKeyInventoryOffset);
    fb.PutUint8(party.GetKeys().GetInventory().GetNumberItems());
    fb.PutUint16LE(party.GetKeys().GetInventory().GetCapacity());
    Save(party.GetKeys().GetInventory(), fb);

    fb.Seek(GameData::sActiveCharactersOffset);
    fb.PutUint8(party.mActiveCharacters.size());
    for (const auto charIndex : party.mActiveCharacters)
        fb.PutUint8(charIndex.mValue);
}

void Save(const WorldClock& worldClock, FileBuffer& fb)
{
    fb.Seek(GameData::sTimeOffset);
    fb.PutUint32LE(worldClock.GetTime().mTime);
    fb.PutUint32LE(worldClock.GetTimeLastSlept().mTime);
}

}
