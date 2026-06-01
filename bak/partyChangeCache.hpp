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

class PartyChangeCache {
public:
    void CacheState(GameState& gs);
    void ClearCondition(const GameState& gs, Condition condition);
    PartyChangeResult CheckPartyChanges(GameState& gs, bool camping, bool inInn);

private:
    std::optional<PartyChangeResult> CheckForDeath(GameState& gameState);
    std::optional<PartyChangeResult> CheckSkillImprovements(GameState& gameState);
    std::optional<PartyChangeResult> CheckNewConditions(GameState& gameState, bool inInn);

    std::array<Conditions, sMaxCharacters> mCachedConditions{};
    std::array<std::uint16_t, sMaxCharacters> mCachedUnseenImprovements{};
};

}

