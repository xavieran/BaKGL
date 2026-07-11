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

struct GridCellInfo
{
    std::optional<glm::vec4> mColor{};
    BAK::EntityIndex mEntityId{0};
    Game::Combat::GridPos mGridPos{0, 0};
};

class ClickableEntity
{
public:
    BAK::EntityType mEntityType;
    BAK::GenericContainer* mContainer;
};

enum class ClipDisplayMode
{
    Vanilla,
    ShowClips,
    OnlyClips
};

class GameRunner : public BAK::IZoneLoader, public Combat::ICombatStage
{
public:
    static constexpr double sMoveDuration = 0.15;
    static constexpr double sFrameTime = 0.25;
    static constexpr float sDamageTextHeightOffset = 250.0f;
    static constexpr float sHitFlashDuration = 1.0f;

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
    void ToggleDisplayAllCells();
    bool IsGridVisible() const { return mGridVisible; }
    void SetClipDisplayMode(ClipDisplayMode mode);
    const std::vector<Renderable>& GetClipRenderables() const { return mClipRenderables; }
    ClipDisplayMode GetClipDisplayMode() const { return mClipDisplayMode; }
    void SetClipEnabled(bool clip) { mClipEnabled = clip; }
    bool GetClipEnabled() const { return mClipEnabled; }
    bool CannotMoveHere(BAK::GamePosition playerPos) const;
    std::optional<BAK::DoorIndex> GetDoorIndex(glm::uvec2 bakLocation) const;
    void SetFollowRoad(bool follow) { mFollowRoad = follow; }
    bool GetFollowRoad() const { return mFollowRoad; }
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

    void FlashCombatant(
        BAK::EntityIndex entityId,
        glm::vec4 color) override;

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
    std::vector<Renderable> mClipRenderables{};
    ClipDisplayMode mClipDisplayMode{ClipDisplayMode::Vanilla};
    std::vector<GridCellInfo> mGridCells{};
    void UpdateGridCellColors();

    bool mAnimationActive{false};
    double mAnimationSpeedMultiplier{1.0};

    bool mClipEnabled{false};
    bool mFollowRoad{false};

    std::vector<BAK::EntityIndex> mHiddenWorldItems{};
    std::unordered_map<BAK::EntityIndex, BAK::EntityType> mEntityTypes{};
    std::unordered_map<glm::uvec2, BAK::DoorIndex, UVec2Hash> mDoorLocations{};


    GlyphStore mGlyphStore;

    std::optional<BAK::EntityIndex> mHoveredEntity;

    const Logging::Logger& mLogger;
};

}
