#pragma once

#include "bak/inventoryItem.hpp"
#include "bak/skills.hpp"

#include "xbak/FileBuffer.h"

#include <optional>
#include <ostream>

namespace BAK {

struct LockStats
{
    unsigned mLockFlag;
    unsigned mRating;
    unsigned mFairyChestIndex;
    unsigned mTrapDamage;
};

LockStats LoadLock(FileBuffer& fb);

struct FairyChest
{
    std::string mAnswer;
    std::vector<std::string> mOptions;
    std::string mHint;
};

std::ostream& operator<<(std::ostream&, const LockStats&);

// This is the lock "image" type
enum class LockType
{
    Easy,
    Medium,
    Hard,
    Unpickable
};

std::string_view ToString(LockType);

LockType ClassifyLock(unsigned lockRating);

std::optional<unsigned> GetLockIndex(unsigned lockRating);
ItemIndex GetCorrespondingKey(unsigned lockIndex);
unsigned DescribeLock(unsigned picklockSkill, unsigned lockRating);

bool TryOpenLockWithKey(const BAK::InventoryItem&, unsigned lockRating); 
bool WouldKeyBreak(const BAK::InventoryItem&, unsigned lockRating);
bool KeyBroken(const InventoryItem& item, unsigned skill, unsigned lockRating);

bool PicklockBroken(unsigned skill, unsigned lockRating);
bool PicklockSkillImproved();
bool CanPickLock(unsigned skill, unsigned lockRating);

FairyChest GenerateFairyChest(const std::string&);

}
