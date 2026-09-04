#pragma once

#include "game/combatModelLoader.hpp"
#include "game/combat/actorStore.hpp"
#include "game/combat/combatManager.hpp"
#include "game/combat/combatStage.hpp"
#include "game/glyphStore.hpp"
#include "game/mapIcons.hpp"
#include "game/encounterHandler.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"
#include "game/movementManager.hpp"
#include "game/zoomManager.hpp"

#include "game/gateAnimator.hpp"

#include "bak/ICameraManager.hpp"
#include "bak/IZoneLoader.hpp"
#include "bak/combat/combat.hpp"
#include "bak/combat/retreat.hpp"
#include "bak/container.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/save/underground.hpp"
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

class GameRunner : public BAK::IZoneLoader, BAK::ICameraManager
{
public:
    GameRunner(
        Camera& partyCamera,
        Camera& viewCamera,
        BAK::GameState& gameState,
        Gui::GuiManager& guiManager,
        bool debugRenderEncounters = false,
        double animationSpeedMultiplier = 1.0,
        bool nonRotatingMap = false);
    
    /* IZoneLoader */
    void DoTeleport(BAK::Encounter::Teleport teleport) override;
    void LoadGame(std::string savePath, std::optional<BAK::Chapter> chapter) override;

    /* ICameraManager */
    void ToggleFollowRoad() override;
    void ShowOverheadView() override;
    void ShowFirstPersonView() override;
    void ZoomOut() override;
    void ZoomIn() override;

    void LoadZoneData(BAK::ZoneNumber zone);
    void DoTransition(
        BAK::ZoneNumber targetZone,
        BAK::GamePositionAndHeading targetLocation);
    void LoadSystems();

    void DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container, BAK::EntityIndex entityIndex);
    bool DoEncounter(const BAK::Encounter::Encounter& encounter);
    bool CheckAndDoEncounter(glm::uvec2 position);
    
    void RunGameUpdate(bool advanceTime);
    void UpdateViewCamera();
    MovementManager& GetMovementManager() { return mMovementManager; }
    void CheckClickable(unsigned entityId);
    void HandleRightClick(unsigned entityId);
    void SetHoveredEntity(std::optional<BAK::EntityIndex> id);

    void ShowGrid();
    void HideGrid();
    void ShowCombatFloor();
    void RemoveUndergroundCombatFloor();
    void ToggleDisplayAllCells();
    bool IsGridVisible() const { return mGridVisible; }
    void SetClipDisplayMode(ClipDisplayMode mode);
    const std::vector<Renderable>& GetClipRenderables() const { return mClipRenderables; }
    ClipDisplayMode GetClipDisplayMode() const { return mClipDisplayMode; }
    void OnDoorStateChanged(BAK::DoorIndex doorIndex, bool isOpen);
    bool IsAnimationActive() const { return mAnimationActive || mCombatStage.IsAnimationActive(); }
    bool InputDisabled() const { return mPitDeathInProgress || IsAnimationActive(); }
    void ToggleUndergroundModels();
    bool HandleGridCellClick(unsigned entityId, bool isRightClick);

    const Graphics::RenderData& GetMapIconsRenderData() const;
    const std::vector<Renderable>& GetPartyMarker() const { return mPartyMarker; }

    void SetupCombatCamera(const BAK::Encounter::Encounter&);
    void RestoreCameraAfterCombat();
    bool IsCombatCameraActive() const { return mCombatCameraActive; }
    void CombatCompleted(BAK::CombatResult);
    void EnterCombatFromEncounter();
    void OnEnterTile(glm::uvec2 tile);
    void MarkVisibleObjects(glm::uvec2 tile);
    void UpdateOverheadVisibility();
    void RestoreFirstPersonVisibility();
    void HideOverheadHidden();
    void ShowOverheadHidden();

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

    void StartGateAnimation();
    void SetCatapultFrame();
    void CheckPitDeath();
    void TriggerPitDeath();
    void AnimateCrossPit(BAK::EntityIndex entityIndex);
    void UpdatePartyMarkerScale(glm::uvec2 orthoDims);
    void UpdatePartyMarker();
    void CheckAndRestoreOverheadView();
    glm::uvec2 GetOrthoViewDimensions() const;
    float GetZoneFieldOfView() const;
    void UpdateOrthoProjection(glm::uvec2 dims);

public:
    Camera& mPartyCamera;
    Camera& mViewCamera;
    bool mNonRotatingMap{false};
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    MovementManager mMovementManager;
    InteractableFactory mInteractableFactory;
    std::unique_ptr<IInteractable> mCurrentInteractable;

    std::unique_ptr<BAK::Zone> mZoneData;

    const BAK::Encounter::Encounter* mActiveEncounter;
    std::unordered_map<BAK::EntityIndex, const BAK::Encounter::Encounter*> mEncounters;
    std::unordered_map<BAK::EntityIndex, ClickableEntity> mClickables{};
    BAK::GenericContainer mNullContainer;
    std::unique_ptr<Systems> mSystems{nullptr};
    BAK::Encounter::TeleportFactory mTeleportFactory{};

    CombatModelLoader mCombatModelLoader{};
    ActorStore mWorldActorStore;
    BAK::GamePositionAndHeading mCombatPlayerPos{};
    GlyphStore mGlyphStore;

    MapIcons mMapIcons;
    std::vector<Renderable> mPartyMarker{};
    BAK::EntityIndex mPartyMarkerId{};
    glm::vec3 mPartyMarkerScale{};

    std::unique_ptr<Graphics::RenderData> mZoneRenderData{};
    EncounterHandler mEncounterHandler;

    std::unordered_map<BAK::CombatIndex, std::vector<BAK::EntityIndex>> mCombatActorIds{};
    Combat::CombatStage mCombatStage;
    Combat::CombatManager mCombatManager;
    bool mClickablesEnabled{};
    bool mDebugRenderEncounters{false};

    glm::vec3 mSavedCameraPos{};
    glm::vec2 mSavedCameraAngle{};
    bool mCombatCameraActive{false};
    bool mWasInOverheadView{false};
    BAK::CardinalDirection mRetreatDirection{};

    bool mGridVisible{false};
    std::vector<Renderable> mGridCellRenderables{};
    std::vector<BAK::EntityIndex> mCombatFloorIds{};
    std::vector<BAK::EntityIndex> mHiddenTunnels{};
    std::vector<Renderable> mClipRenderables{};
    ClipDisplayMode mClipDisplayMode{ClipDisplayMode::Vanilla};
    std::vector<GridCellInfo> mGridCells{};
    void UpdateGridCellColors();

    ZoomManager mZoomManager{};

    bool mAnimationActive{false};
    bool mPitDeathInProgress{false};

    std::vector<BAK::EntityIndex> mHiddenWorldItems{};
    std::unordered_map<BAK::EntityIndex, glm::uvec2> mPitLocations{};
    std::unordered_map<BAK::EntityIndex, BAK::EntityType> mEntityTypes{};
    DoorLocationMap mDoorLocations{};
    std::unordered_map<BAK::DoorIndex, BAK::EntityIndex> mDoorIndexToEntityId{};

    std::unordered_map<BAK::EntityIndex, Graphics::MeshObjectStorage::OffsetAndLength> mMainViewOffsets{};
    std::unordered_map<BAK::EntityIndex, Graphics::MeshObjectStorage::OffsetAndLength> mOverheadViewOffsets{};

    struct TileObjectRef
    {
        glm::uvec2 mTile{};
        unsigned mLocalIndex{};
    };
    std::unordered_map<BAK::EntityIndex, TileObjectRef> mTileObjectRefs{};
    Graphics::MeshObjectStorage::OffsetAndLength mUndergroundDoorClosed;
    Graphics::MeshObjectStorage::OffsetAndLength mUndergroundDoorOpen;
    bool mShowUnderground{false};

    using FrameOffsets = std::vector<Graphics::MeshObjectStorage::OffsetAndLength>;
    std::unordered_map<BAK::EntityIndex, const FrameOffsets*> mAnimatedEntities{};
    std::unordered_map<std::string, FrameOffsets> mAnimatedModelFrames{};

    std::optional<BAK::EntityIndex> mGateEntity{};
    GateAnimator* mGateAnimator{nullptr};

    std::optional<BAK::EntityIndex> mCatapultEntity{};

    std::optional<BAK::EntityIndex> mHoveredEntity;

    glm::uvec2 mCurrentTile{};

    const Logging::Logger& mLogger;
};

}
