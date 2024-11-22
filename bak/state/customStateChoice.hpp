#pragma once

namespace BAK {
class GameState;
};

namespace BAK::State {

class CustomStateEvaluator
{
public:
    CustomStateEvaluator(const GameState&);

    bool AnyCharacterStarving() const;
    bool Plagued() const;
    bool HaveSixSuitsOfArmor() const;
    bool AllPartyArmorIsGoodCondition() const;
    bool PoisonedDelekhanArmyChests() const;
    bool AnyCharacterSansWeapon() const;
    bool AnyCharacterHasNegativeCondition() const;
    bool AnyCharacterIsUnhealthy() const;
    bool AllCharactersHaveNapthaMask() const;
    bool NormalFoodInArlieChest() const;
    bool PoisonedFoodInArlieChest() const;

private:
    const GameState& mGameState;
};

}
