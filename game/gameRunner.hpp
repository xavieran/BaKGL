#pragma once

#include "game/combatModelLoader.hpp"
#include "game/combat/actorStore.hpp"
#include "game/combat/combatManager.hpp"
#include "game/combat/combatStage.hpp"
#include "game/glyphStore.hpp"
#include "game/encounterHandler.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"
#include "game/movementManager.hpp"

#include "game/gateAnimator.hpp"

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

class GameRunner : public BAK::IZoneLoader
{
public:
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

    void DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container, BAK::EntityIndex entityIndex);
    bool CheckAndDoEncounter(glm::uvec2 position);
    
    void RunGameUpdate(bool advanceTime);
    MovementManager& GetMovementManager() { return mMovementManager; }
    void CheckClickable(unsigned entityId);
    void HandleRightClick(unsigned entityId);
    void SetHoveredEntity(std::optional<BAK::EntityIndex> id);

    void ShowGrid();
    void HideGrid();
    void ToggleDisplayAllCells();
    bool IsGridVisible() const { return mGridVisible; }
    void SetClipDisplayMode(ClipDisplayMode mode);
    const std::vector<Renderable>& GetClipRenderables() const { return mClipRenderables; }
    ClipDisplayMode GetClipDisplayMode() const { return mClipDisplayMode; }
    void OnDoorStateChanged(BAK::DoorIndex doorIndex, bool isOpen);
    bool IsAnimationActive() const { return mAnimationActive || mCombatStage.IsAnimationActive(); }
    bool InputDisabled() const { return mPitDeathInProgress || IsAnimationActive(); }
    bool HandleGridCellClick(unsigned entityId, bool isRightClick);

    void SetupCombatCamera(const BAK::Encounter::Encounter&);
    void RestoreCameraAfterCombat();
    void CombatCompleted(BAK::CombatResult);
    void EnterCombatFromEncounter();

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

public:
    Camera& mCamera;
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

    std::unique_ptr<Graphics::RenderData> mZoneRenderData{};
    EncounterHandler mEncounterHandler;

    std::unordered_map<BAK::CombatIndex, std::vector<BAK::EntityIndex>> mCombatActorIds{};
    Combat::CombatStage mCombatStage;
    Combat::CombatManager mCombatManager;
    bool mClickablesEnabled{};
    bool mDebugRenderEncounters{false};

    glm::vec3 mSavedCameraPos{};
    glm::vec2 mSavedCameraAngle{};
    BAK::CardinalDirection mRetreatDirection{};

    bool mGridVisible{false};
    std::vector<Renderable> mGridCellRenderables{};
    std::vector<Renderable> mClipRenderables{};
    ClipDisplayMode mClipDisplayMode{ClipDisplayMode::Vanilla};
    std::vector<GridCellInfo> mGridCells{};
    void UpdateGridCellColors();

    bool mAnimationActive{false};
    bool mPitDeathInProgress{false};

    std::vector<BAK::EntityIndex> mHiddenWorldItems{};
    std::unordered_map<BAK::EntityIndex, glm::uvec2> mPitLocations{};
    std::unordered_map<BAK::EntityIndex, BAK::EntityType> mEntityTypes{};
    DoorLocationMap mDoorLocations{};
    std::unordered_map<BAK::DoorIndex, BAK::EntityIndex> mDoorIndexToEntityId{};

    using FrameOffsets = std::vector<Graphics::MeshObjectStorage::OffsetAndLength>;
    std::unordered_map<BAK::EntityIndex, const FrameOffsets*> mAnimatedEntities{};
    std::unordered_map<std::string, FrameOffsets> mAnimatedModelFrames{};

    std::optional<BAK::EntityIndex> mGateEntity{};
    GateAnimator* mGateAnimator{nullptr};

    std::optional<BAK::EntityIndex> mCatapultEntity{};

    std::optional<BAK::EntityIndex> mHoveredEntity;

    const Logging::Logger& mLogger;
};

}
