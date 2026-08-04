#include "game/combat/combatStage.hpp"

#include "game/combat/flashAnimator.hpp"
#include "game/combat/types.hpp"
#include "game/combat/frameAnimator.hpp"
#include "game/combat/moveAnimator.hpp"
#include "game/textAnimator.hpp"
#include "game/systems.hpp"

#include "bak/coordinates.hpp"

#include "gui/IGuiManager.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <utility>

namespace Game::Combat {

CombatStage::CombatStage(
    Gui::IGuiManager& guiManager,
    const Camera& camera,
    const GlyphStore& glyphStore,
    const CombatModelLoader& combatModelLoader,
    const BAK::GamePositionAndHeading& combatPlayerPos,
    double animationSpeedMultiplier)
:
    mGuiManager{guiManager},
    mCamera{camera},
    mGlyphStore{glyphStore},
    mCombatModelLoader{combatModelLoader},
    mCombatPlayerPos{combatPlayerPos},
    mCombatActorStore{mCombatModelLoader, nullptr},
    mAnimationSpeedMultiplier{animationSpeedMultiplier},
    mLogger{Logging::LogState::GetLogger("Game::Combat::CombatStage")}
{}

void CombatStage::SetCombatManager(BAK::ICombatManager* combatManager)
{
    mCombatManager = combatManager;
}

void CombatStage::SetSystems(Systems* systems)
{
    mSystems = systems;
    mCombatActorStore.SetSystems(systems);
}

void CombatStage::OnTimeDelta(double timeDelta)
{
    for (auto& actor : mCombatActorStore.GetActors())
    {
        actor.AdvanceIdle(timeDelta);
    }
}

Actor* CombatStage::GetActor(BAK::EntityIndex id)
{
    auto* actor = mCombatActorStore.GetActor(id);
    assert(actor);
    return actor;
}

void CombatStage::MoveCombatant(
    BAK::EntityIndex entityId,
    glm::uvec2 sourceGrid,
    glm::uvec2 targetGrid)
{
    auto* actor = GetActor(entityId);
    auto bakPos = BAK::MakeGamePositionFromGridCell(mCombatPlayerPos, targetGrid);
    auto targetPos = BAK::ToGlCoord<float>(bakPos);
    mLogger.Debug() << "Moving combatant: " << entityId
        << " cam: " << mCombatPlayerPos << " pos: " << bakPos << "\n";

    auto startPos = actor->mLocation;

    auto direction = BAK::GetDirectionBetween(sourceGrid, targetGrid);
    actor->SetDirection(direction);

    auto moveDuration = sMoveDuration * mAnimationSpeedMultiplier;

    mAnimationActive = true;
    mGuiManager.AddAnimator(
        std::make_unique<MoveAnimator>(
            *actor,
            startPos,
            targetPos,
            moveDuration,
            [this, targetGrid]() mutable {
                ASSERT(mCombatManager);
                mAnimationActive = false;
                mCombatManager->CompleteMove(GridPos(targetGrid));
            }));
}

void CombatStage::SetCombatantAction(
    BAK::EntityIndex entityId,
    BAK::AnimationType animType)
{
    auto* actor = GetActor(entityId);
    mLogger.Spam() << "Setting combatant action: " << entityId
        << " mid: " << actor->mMonster
        << " anim: " << ToString(animType) << "\n";
    actor->StartAnimation(animType);
}

void CombatStage::SetCombatantDirection(
    BAK::EntityIndex entityId,
    BAK::Direction direction)
{
    auto* actor = GetActor(entityId);
    mLogger.Spam() << "Setting combatant direction: " << entityId
        << " dir: " << static_cast<unsigned>(direction) << "\n";
    actor->SetDirection(direction);
}

void CombatStage::SetCombatantUpdateIdle(
    BAK::EntityIndex entityId,
    bool update)
{
    auto* actor = GetActor(entityId);
    actor->SetUpdateIdle(update);
}

void CombatStage::AnimateCombatant(
    BAK::EntityIndex entityId)
{
    AnimateCombatant(entityId, []{});
}

void CombatStage::AnimateCombatant(
    BAK::EntityIndex entityId,
    std::function<void()> onFinished)
{
    auto* actor = GetActor(entityId);
    mLogger.Debug() << "Animating combatant (with callback): " << entityId
        << " mid: " << actor->mMonster << "\n";

    assert(BAK::IsCardinal(actor->mDirection));

    auto frameTime = sFrameTime * mAnimationSpeedMultiplier;

    mAnimationActive = true;
    mGuiManager.AddAnimator(
        std::make_unique<FrameAnimator>(
            *actor,
            frameTime,
            [this, finished=std::move(onFinished)]() mutable {
                mAnimationActive = false;
                finished();
            }));
}

void CombatStage::AnimateAttack(
    BAK::EntityIndex entityId,
    glm::uvec2 targetGrid)
{
    auto* actor = GetActor(entityId);
    mLogger.Debug() << "Animating attack: " << entityId
        << " mid: " << actor->mMonster << "\n";
    assert(BAK::IsCardinal(actor->mDirection));

    auto frameTime = sFrameTime * mAnimationSpeedMultiplier;

    mAnimationActive = true;
    mGuiManager.AddAnimator(
        std::make_unique<FrameAnimator>(
            *actor,
            frameTime,
            [this, targetGrid]() mutable {
                ASSERT(mCombatManager);
                mAnimationActive = false;
                mCombatManager->CompleteAttack(GridPos(targetGrid));
            }));
}

void CombatStage::CombatFinished(BAK::CombatResult result)
{
    // Yuck... we probably don't need to go in and then back
    // here to exit combat...
    mGuiManager.ExitCombat(result);
}

void CombatStage::DisplayText(
    BAK::EntityIndex target,
    std::string text,
    TextColor color)
{
    auto* actor = GetActor(target);

    auto worldPos = actor->mLocation;
    worldPos.y += sDamageTextHeightOffset;

    ASSERT(mSystems);
    mGuiManager.AddAnimator(
        std::make_unique<TextAnimator>(
            *mSystems,
            mGlyphStore,
            mCamera,
            worldPos,
            text,
            color,
            sHitFlashDuration));
}

void CombatStage::FlashCombatant(BAK::EntityIndex entityId, glm::vec4 color)
{
    auto* actor = GetActor(entityId);
    mGuiManager.AddAnimator(
        std::make_unique<FlashAnimator>(*actor, color));
}

}
