#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"
#include "bak/combat/combat.hpp"
#include "bak/combat/combatModel.hpp"

#include <functional>
#include <glm/glm.hpp>

namespace Game::Combat {

class ICombatStage
{
public:
    virtual void MoveCombatant(
        BAK::EntityIndex,
        glm::uvec2 sourceGrid,
        glm::uvec2 targetGrid) = 0;

    virtual void SetCombatantAction(
        BAK::EntityIndex,
        BAK::AnimationType animType) = 0;

    virtual void SetCombatantDirection(
        BAK::EntityIndex,
        BAK::Direction) = 0;

    virtual void SetCombatantUpdateIdle(
        BAK::EntityIndex,
        bool) = 0;

    virtual void AnimateCombatant(
        BAK::EntityIndex) = 0;

    virtual void AnimateCombatant(
        BAK::EntityIndex,
        std::function<void()>) = 0;

    virtual void AnimateAttack(
        BAK::EntityIndex,
        glm::uvec2 targetGrid) = 0;

    virtual void CombatFinished(
        BAK::CombatResult) = 0;

    virtual ~ICombatStage() = default;
};

}
