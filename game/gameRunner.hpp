#pragma once

#include "game/combatModelLoader.hpp"
#include "game/combat/actorStore.hpp"
#include "game/combat/combatManager.hpp"
#include "game/combat/ICombatStage.hpp"
#include "game/glyphStore.hpp"
#include "game/textAnimator.hpp"
#include "game/encounterHandler.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/IZoneLoader.hpp"
#include "bak/combat/combat.hpp"
#include "bak/combat/retreat.hpp"
#include "bak/container.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/types.hpp"

#include "graphics/renderData.hpp"
#include "graphics/renderer.hpp"

#include "com/logger.hpp"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

class Camera;
namespace BAK {
class CombatWorldLocation;
class GameState;
class Zone;
namespace Encounter {
class Encounter;
}
}

namespace Gui {
class GuiManager;
}

namespace Game {

class ClickableEntity
{
public:
    BAK::EntityType mEntityType;
    BAK::GenericContainer* mContainer;
};

class GameRunner : public BAK::IZoneLoader, public Combat::ICombatStage
{
public:
    static constexpr double sMoveDuration = 0.15;
    static constexpr double sFrameTime = 0.25;
    static constexpr float sDamageTextHeightOffset = 250.0f;

    GameRunner(
        Camera& camera,
        BAK::GameState& gameState,
        Gui::GuiManager& guiManager,
        bool debugRenderEncounters = false,
        double animationSpeedMultiplier = 1.0);
    
    void DoTeleport(BAK::Encounter::Teleport teleport) override;
    void LoadGame(std::string savePath, std::optional<BAK::Chapter> chapter) override;

    void LoadZoneData(BAK::ZoneNumber zone);
    void DoTransition(
        BAK::ZoneNumber targetZone,
        BAK::GamePositionAndHeading targetLocation);
    void LoadSystems();

    void DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container);
    bool CheckAndDoEncounter(glm::uvec2 position);
    
    void RunGameUpdate(bool advanceTime);
    void CheckClickable(unsigned entityId);
    void SetHoveredEntity(std::optional<BAK::EntityIndex> id);

    void ShowGrid();
    void HideGrid();
    bool IsGridVisible() const { return mGridVisible; }
    bool IsAnimationActive() const { return mAnimationActive; }
    bool HandleGridCellClick(unsigned entityId, bool isRightClick);

    void SetupCombatCamera(const BAK::Encounter::Encounter&);
    void RestoreCameraAfterCombat();
    void CombatCompleted(BAK::CombatResult);
    void EnterCombatFromEncounter();

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
        BAK::CombatResult) override;

    void DisplayText(
        BAK::EntityIndex target,
        std::string text,
        TextColor color) override;

    const Graphics::RenderData& GetZoneRenderData() const;
    void OnTimeDelta(double timeDelta);

    void LoadTileActors(std::uint8_t tileIndex);
    void UnloadWorldActors();
    void LoadWorldActors();
    void ClearCombatActors();
    void CleanCombatsOnNewZone();
    const BAK::Encounter::Encounter& FindEncounterByCombatIndex(
        BAK::CombatIndex combatIndex) const;

private:
    static Game::Combat::GridPos IndexToGridPos(unsigned i)
    {
        return Game::Combat::GridPos{
            static_cast<int>(i % BAK::gCombatGridCols),
            static_cast<int>(i / BAK::gCombatGridCols)};
    }

public:
    Camera& mCamera;
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    InteractableFactory mInteractableFactory;
    std::unique_ptr<IInteractable> mCurrentInteractable;

    std::unique_ptr<BAK::Zone> mZoneData;

    const BAK::Encounter::Encounter* mActiveEncounter;
    std::unordered_map<BAK::EntityIndex, const BAK::Encounter::Encounter*> mEncounters;
    std::unordered_map<BAK::EntityIndex, ClickableEntity> mClickables{};
    BAK::GenericContainer mNullContainer;
    std::unique_ptr<Systems> mSystems{nullptr};
    BAK::Encounter::TeleportFactory mTeleportFactory{};
    ActorStore mCombatActorStore;
    ActorStore mWorldActorStore;

    std::unique_ptr<Graphics::RenderData> mZoneRenderData{};
    CombatModelLoader mCombatModelLoader{};
    EncounterHandler mEncounterHandler;

    std::unordered_map<BAK::CombatIndex, std::vector<BAK::EntityIndex>> mCombatActorIds{};
    Combat::CombatManager mCombatManager;
    bool mClickablesEnabled{};
    bool mDebugRenderEncounters{false};

    glm::vec3 mSavedCameraPos{};
    glm::vec2 mSavedCameraAngle{};
    BAK::CardinalDirection mRetreatDirection{};
    BAK::GamePositionAndHeading mCombatPlayerPos{};

    bool mGridVisible{false};
    std::vector<Renderable> mGridCellRenderables{};
    std::vector<BAK::EntityIndex> mGridCellEntityIds{};

    bool mAnimationActive{false};

    double mAnimationSpeedMultiplier{1.0};

    GlyphStore mGlyphStore;

    std::optional<BAK::EntityIndex> mHoveredEntity;

    const Logging::Logger& mLogger;
};

}
