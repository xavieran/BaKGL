#include "bak/lock.hpp"

#include "bak/random.hpp"

#include "com/ostream.hpp"
#include "com/string.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const LockStats& lock)
{
    os << "Lock { flag: " << lock.mLockFlag << " rating: " << lock.mRating
        << " fairy: " << lock.mFairyChestIndex << " damage: " << lock.mTrapDamage << "}";
    return os;
}


LockStats LoadLock(FileBuffer& fb)
{
    const auto lockFlag = fb.GetUint8();
    const auto picklock = fb.GetUint8();
    const auto fairyChestIndex = fb.GetUint8();
    const auto damage = fb.GetUint8();
    return LockStats{lockFlag, picklock, fairyChestIndex, damage};
}

std::ostream& operator<<(std::ostream& os, const Door& door)
{
    os << "Door { index: " << door.mDoorIndex << "}"; 
    return os;
}

LockType ClassifyLock(unsigned lockRating)
{
    if (lockRating < 0x33)
        return LockType::Easy;
    else if (lockRating < 0x51)
        return LockType::Medium;
    else if (lockRating < 0x65)
        return LockType::Hard;
    else
        return LockType::Unpickable;
}

LockType ClassifyDoor(unsigned lockRating)
{
    if (lockRating < 50)
        return LockType::Easy;
    else if (lockRating < 80)
        return LockType::Medium;
    else if (lockRating < 100)
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
        && (GetCorrespondingKey(*lockIndex) == item.GetItemIndex());
}

bool WouldKeyBreak(const InventoryItem& item, unsigned lockRating)
{
    // Special keys should never break
    return !TryOpenLockWithKey(item, lockRating) && GetKeyRating(item.GetItemIndex()) <= 100;
}

unsigned DescribeLock(unsigned picklockSkill, unsigned lockRating)
{
    if (lockRating > 106)
        return 3; // lock is broken
    else if (lockRating > 100 && lockRating < 107)
        return 2; // lock takes special key
    else if (picklockSkill > lockRating)
        return 0; // lock is easy
    else if (picklockSkill <= lockRating)
        return 1; // lock too complicated
    else
        ASSERT(false);
        return 0;
}

unsigned GetRandomMod100()
{
    return (GetRandom() & 0xfff) % 100;
}

bool KeyBroken(const InventoryItem& item, unsigned skill, unsigned lockRating)
{
    // Returns e.g. 0x32 for peasants key...
    const auto keyRating = GetKeyRating(item.GetItemIndex());
    // from the disassembly... looks reasonable
    const auto diff = 100 - keyRating;
    const auto skillDiv3 = skill / 3;
    const auto diffSubSkill = (diff - skillDiv3) << 1;
    const auto diffSubSkillDiv3 = diffSubSkill / 3;

    const auto randomNumber = GetRandomMod100();
    return randomNumber < diffSubSkillDiv3;
}

bool PicklockBroken(unsigned skill, unsigned lockRating)
{
    const auto randomNumber = GetRandomMod100();
    // Returns e.g. 0x32 for peasants key...
    const auto diff = ((lockRating - skill) << 1) / 3;

    return randomNumber < diff;
}

bool PicklockSkillImproved()
{
    return GetRandomMod100() < 0x28;
}

bool CanPickLock(unsigned skill, unsigned lockRating)
{
    return (lockRating <= 100) && (skill > lockRating);
}

FairyChest GenerateFairyChest(const std::string& data)
{
    const auto texts = SplitString("\n#", data);
    ASSERT(texts.size() == 3);
    const auto& answer = texts[0];

    auto options = SplitString("\n", texts[1]);
    options.erase(options.begin()); // remove leading empty string

    const auto& hint = texts[2];

    for (unsigned i = 0; i < options.size(); i++)
        ASSERT(options[i].size() == answer.size());

    return FairyChest{answer, options, hint};
}

}
