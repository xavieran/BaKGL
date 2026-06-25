#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"
#include "bak/combat/combatModel.hpp"

#include <glm/glm.hpp>

namespace Game::Combat {

class ICombatStage
{
public:
    virtual void MoveCombatant(
        BAK::EntityIndex entityId,
        glm::uvec2 sourceGrid,
        glm::uvec2 targetGrid) = 0;

    virtual void SetCombatantAction(
        BAK::EntityIndex entityId,
        BAK::AnimationType animType) = 0;

    virtual void SetCombatantDirection(
        BAK::EntityIndex entityId,
        BAK::Direction direction) = 0;

    virtual void AnimateCombatant(
        BAK::EntityIndex entityId) = 0;

    virtual void AnimateAttack(
        BAK::EntityIndex entityId,
        glm::uvec2 targetGrid) = 0;

    virtual ~ICombatStage() = default;
};

}
