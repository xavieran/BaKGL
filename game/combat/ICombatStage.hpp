#pragma once

#include "bak/types.hpp"
#include "bak/combat/combatModel.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace Game::Combat {

class ICombatStage
{
public:
    virtual void MoveCombatant(
        BAK::EntityIndex entityId,
        glm::uvec2 sourceGrid,
        glm::uvec2 targetGrid,
        std::function<void()>&& onComplete) = 0;

    virtual void SetCombatantAction(
        BAK::EntityIndex entityId,
        BAK::AnimationType animType) = 0;

    virtual void AnimateCombatant(
        BAK::EntityIndex entityId,
        std::function<void()>&& onComplete) = 0;

    virtual ~ICombatStage() = default;
};

}
