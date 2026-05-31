#pragma once

#include "bak/condition.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/types.hpp"

#include <array>

namespace BAK {

class Conditions;
class GameState;

struct PartyChangeResult {
    bool mChanged = false;
    Target mDialog{KeyTarget{0}};
    bool mDead = false;
};

struct PartyChangeCache {
    std::array<Conditions, sMaxCharacters> mCachedConditions{};
    std::array<std::uint16_t, sMaxCharacters> mCachedUnseenImprovements{};
};

void CachePartyState(PartyChangeCache& cache, const GameState&);
PartyChangeResult CheckPartyChanges(PartyChangeCache& cache, GameState&);

}
