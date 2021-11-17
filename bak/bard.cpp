#include "bak/bard.hpp"
#include "bak/dialogSources.hpp"

namespace BAK::Bard {

BardStatus ClassifyBardAttempt(unsigned bardingSkill, unsigned innRequirement)
{
    const auto xpGained = 128 - (112 * (bardingSkill / 100.));
    if (bardingSkill < innRequirement)
    {
        if (bardingSkill < ((innRequirement * 3) / 4))
        {
            return BardStatus::Failed;
        }
        else
        {
            return BardStatus::Poor;
        }
    }
    else
    {
        if (bardingSkill < ((innRequirement + 100) / 2))
        {
            return BardStatus::Good;
        }
        else
        {
            return BardStatus::Best;
        }
    }
}

Royals GetReward(BardStatus status, Sovereigns innReward, Chapter chapter)
{
    const auto perChapterReward = (chapter.mValue - 1) * .05 * GetRoyals(innReward).mValue;
    const auto chapterReward = static_cast<unsigned>(std::round(perChapterReward));
    const unsigned reward = GetRoyals(innReward).mValue + chapterReward;
    switch (status)
    {
    case BardStatus::Failed: return Royals{0};
    case BardStatus::Poor:   return Royals{reward / 4};
    case BardStatus::Good:   return Royals{reward / 2};
    case BardStatus::Best:   return Royals{reward};
    default: ASSERT(false); return Royals{0};
    }
}


void ReduceAvailableReward(ShopStats& stats, Royals reward)
{
    if (reward.mValue > stats.mBardingReward)
        stats.mBardingReward = 0;
    else
        stats.mBardingReward -= reward.mValue;
}

KeyTarget GetDialog(BardStatus status)
{
    switch (status)
    {
    case BardStatus::Failed: return BAK::DialogSources::mBardingBad;
    case BardStatus::Poor:   return BAK::DialogSources::mBardingPoor;
    case BardStatus::Good:   return BAK::DialogSources::mBardingOkay;
    case BardStatus::Best:   return BAK::DialogSources::mBardingGood;
    default: ASSERT(false); return KeyTarget{0};
    }
}

SongIndex GetSong(BardStatus status)
{
    switch (status)
    {
    case BardStatus::Failed: return SongIndex{0x3f0};
    case BardStatus::Poor:   return SongIndex{0x410};
    case BardStatus::Good:   return SongIndex{0x40f};
    case BardStatus::Best:   return SongIndex{0x3ef};
    default: ASSERT(false); return SongIndex{0};
    }
}
}
