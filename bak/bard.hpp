#pragma once

#include "bak/dialog.hpp"
#include "bak/money.hpp"
#include "bak/types.hpp"

namespace BAK::Bard {

enum class BardStatus
{
    Failed,
    Poor,
    Good,
    Best
};

BardStatus ClassifyBardAttempt(
    unsigned bardingSkill,
    unsigned innRequirement);

Royals GetReward(BardStatus, Sovereigns innReward, Chapter);

KeyTarget GetDialog(BardStatus);

SongIndex GetSong(BardStatus);


}
