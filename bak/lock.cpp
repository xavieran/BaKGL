#include "bak/lock.hpp"

namespace BAK {


std::ostream& operator<<(std::ostream& os, const LockStats& lock)
{
    os << "Lock { flag: " << lock.mLockFlag << " rating: " << lock.mRating
        << " fairy: " << lock.mFairyChestIndex << " damage: " << lock.mTrapDamage << "}";
    return os;
}

LockType ClassifyLock(unsigned lockRating)
{
    if (lockRating < 51)
        return LockType::Easy;
    else if (lockRating < 81)
        return LockType::Medium;
    else if (lockRating < 101)
        return LockType::Hard;
    else
        return LockType::Unpickable;
}

std::string_view ToString(LockType lock)
{
    switch (lock)
    {
    case BAK::LockType::Easy:   return "Easy";
    case BAK::LockType::Medium: return "Medium";
    case BAK::LockType::Hard:   return "Hard";
    case BAK::LockType::Unpickable: return "Unpickable";
    default: return "UnknownLockType";
    }
}

static constexpr auto sKeyItemIndexOffset = 60; // keys start at 60;
static constexpr auto sLockIndices = std::array<std::uint8_t, 12>{
    0x0, 0x32, 0x5a, 0x65, 0x66, 0x67, 0x68, 0x46, 0x3C, 0x50, 0x69, 0x6A};

std::optional<unsigned> GetLockIndex(unsigned lockRating)
{
    const auto it = std::find(sLockIndices.begin(), sLockIndices.end(), lockRating);
    if (it == sLockIndices.end())
        return std::optional<unsigned>{};
    else
        return std::distance(sLockIndices.begin(), it);
}

unsigned GetKeyRating(const BAK::ItemIndex& key)
{
    ASSERT(key.mValue > 60);
    const auto keyIndex = key.mValue - sKeyItemIndexOffset;
    ASSERT(keyIndex <= sLockIndices.size());
    return sLockIndices[keyIndex];
}

ItemIndex GetCorrespondingKey(unsigned lockIndex)
{
    return ItemIndex{lockIndex + sKeyItemIndexOffset};
}

bool TryOpenLockWithKey(const BAK::InventoryItem& item, unsigned lockRating)
{
    const auto lockIndex = GetLockIndex(lockRating);
    return lockIndex 
        && (GetCorrespondingKey(*lockIndex) == item.mItemIndex);
}

bool WouldKeyBreak(const InventoryItem& item, unsigned lockRating)
{
    // Special keys should never break
    return !TryOpenLockWithKey(item, lockRating) && lockRating <= 100;
}

unsigned DescribeLock(unsigned picklockSkill, unsigned lockRating)
{
    if (picklockSkill > lockRating)
        return 0; // lock is easy
    else if (picklockSkill <= lockRating)
        return 1; // lock too complicated
    else if (lockRating > 100 && lockRating < 107)
        return 2; // lock takes special key
    else
        return 3; // lock is broen
}

unsigned GetRandom()
{
    return 5;
}

unsigned GetRandomMod100()
{
    return (GetRandom() & 0xfff) % 100;
}

bool KeyBroken(const InventoryItem& item, const Skill& skill, unsigned lockRating)
{
    // Returns e.g. 0x32 for peasants key...
    const auto keyRating = GetKeyRating(item.mItemIndex);
    const auto diff = 100 - keyRating;
    const auto skillLimitOn3 = skill.mCurrent / 3;
    const auto diffSubSkill = (diff - skillLimitOn3) << 1;
    const auto diffSubSkillOn3 = diffSubSkill / 3;

    const auto randomNumber = GetRandomMod100();
    return randomNumber < diffSubSkillOn3;
}

bool PicklockBroken(const Skill& skill, unsigned lockRating)
{
    const auto randomNumber = GetRandomMod100();
    // Returns e.g. 0x32 for peasants key...
    const auto diff = ((lockRating - skill.mCurrent) << 1) / 3;

    return randomNumber < diff;
}

bool PicklockSkillImproved()
{
    return GetRandomMod100() < 0x28;
}

bool CanPickLock(const Skill& skill, unsigned lockRating)
{
    return (lockRating <= 100) && (skill.mCurrent > lockRating);
}

}
