#pragma once

#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/IZoneLoader.hpp"
#include "bak/container.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/types.hpp"

#include "graphics/renderData.hpp"

#include "com/logger.hpp"

#include "gui/IDialogScene.hpp"

#include <optional>

class Camera;
namespace BAK {
class GameState;
class GameData;
class Zone;
namespace Encounter {
struct Block;
class Combat;
struct Dialog;
class Encounter;
struct EventFlag;
struct GDSEntry;
class Zone;
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

class GameRunner : public BAK::IZoneLoader
{
public:
    GameRunner(
        Camera& camera,
        BAK::GameState& gameState,
        Gui::GuiManager& guiManager);
    
    void DoTeleport(BAK::Encounter::Teleport teleport) override;
    void LoadGame(std::string savePath, std::optional<BAK::Chapter> chapter) override;

    void LoadZoneData(BAK::ZoneNumber zone);
    void DoTransition(
        BAK::ZoneNumber targetZone,
        BAK::GamePositionAndHeading targetLocation);
    void LoadSystems();

    void DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container);
    // Returns <combatActive, combatScouted>
    std::pair<bool, bool> CheckCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);
    void CheckAndDoCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);
    void DoBlockEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Block& block);
    void DoEventFlagEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::EventFlag& flag);
    void DoZoneEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Zone& zone);
    void DoDialogEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Dialog& dialog);
    void DoGDSEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::GDSEntry& gds);
    void DoEncounter(const BAK::Encounter::Encounter& encounter);
    void CheckAndDoEncounter(glm::uvec2 position);
    
    void RunGameUpdate();
    void CheckClickable(unsigned entityId);

    const Graphics::RenderData& GetZoneRenderData() const;
    
    Camera& mCamera;
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    InteractableFactory mInteractableFactory;
    std::unique_ptr<IInteractable> mCurrentInteractable;
    Gui::DynamicDialogScene mDynamicDialogScene;

    std::unique_ptr<BAK::GameData> mGameData;
    std::unique_ptr<BAK::Zone> mZoneData;

    const BAK::Encounter::Encounter* mActiveEncounter;
    std::unordered_map<BAK::EntityIndex, const BAK::Encounter::Encounter*> mEncounters;
    std::unordered_map<BAK::EntityIndex, ClickableEntity> mClickables{};
    BAK::GenericContainer mNullContainer;
    std::unique_ptr<Systems> mSystems;
    glm::vec2 mSavedAngle;
    std::function<void(const BAK::Zone&)> mLoadRenderer;
    BAK::Encounter::TeleportFactory mTeleportFactory;

    std::unique_ptr<Graphics::RenderData> mZoneRenderData{};

    bool mClickablesEnabled;

    const Logging::Logger& mLogger;
};

}
