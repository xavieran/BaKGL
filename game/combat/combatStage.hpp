#pragma once

#include "game/combat/ICombatStage.hpp"
#include "game/combat/actorStore.hpp"
#include "game/combatModelLoader.hpp"

#include "bak/coordinates.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

#include <functional>
#include <string>

class Systems;

namespace Game {
class GlyphStore;
}

namespace Gui {
class IGuiManager;
}

namespace BAK {
class ICombatManager;
}

namespace Game::Combat {

class CombatStage : public ICombatStage
{
public:
    static constexpr double sMoveDuration = 0.15;
    static constexpr double sFrameTime = 0.25;
    static constexpr float sDamageTextHeightOffset = 250.0f;
    static constexpr float sHitFlashDuration = 1.0f;

    CombatStage(
        Gui::IGuiManager& guiManager,
        const GlyphStore& glyphStore,
        const CombatModelLoader& combatModelLoader,
        const BAK::GamePositionAndHeading& combatPlayerPos,
        double animationSpeedMultiplier);

    void MoveCombatant(
        BAK::EntityIndex entityId,
        glm::uvec2 sourceGrid,
        glm::uvec2 targetGrid) override;

    void SetCombatantAction(
        BAK::EntityIndex entityId,
        BAK::AnimationType animType) override;

    void SetCombatantDirection(
        BAK::EntityIndex entityId,
        BAK::Direction direction) override;

    void SetCombatantUpdateIdle(
        BAK::EntityIndex entityId,
        bool update) override;

    void AnimateCombatant(
        BAK::EntityIndex entityId) override;

    void AnimateCombatant(
        BAK::EntityIndex entityId,
        std::function<void()> onFinished) override;

    void AnimateAttack(
        BAK::EntityIndex entityId,
        glm::uvec2 targetGrid) override;

    void CombatFinished(
        BAK::CombatResult result) override;

    void DisplayText(
        BAK::EntityIndex target,
        std::string text,
        TextColor color) override;

    void FlashCombatant(
        BAK::EntityIndex entityId,
        glm::vec4 color) override;

    void SetCombatManager(BAK::ICombatManager* combatManager);
    void SetSystems(Systems* systems);
    void OnTimeDelta(double timeDelta);
    ActorStore& GetActorStore() { return mCombatActorStore; }
    bool IsAnimationActive() const { return mAnimationActive; }

private:
    Actor* GetActor(BAK::EntityIndex id);

    Gui::IGuiManager& mGuiManager;
    const GlyphStore& mGlyphStore;
    const CombatModelLoader& mCombatModelLoader;
    const BAK::GamePositionAndHeading& mCombatPlayerPos;
    BAK::ICombatManager* mCombatManager{nullptr};
    Systems* mSystems{nullptr};
    ActorStore mCombatActorStore;
    bool mAnimationActive{false};
    double mAnimationSpeedMultiplier{1.0};
    const Logging::Logger& mLogger;
};

}
