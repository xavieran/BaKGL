#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"
#include "com/logger.hpp"
#include "game/combatEncounterHandler.hpp"
#include "gui/IDialogScene.hpp"

#include <glm/glm.hpp>

#include <functional>

class Camera;

namespace BAK {
class GameState;
namespace Encounter {
class Encounter;
struct Block;
struct Dialog;
struct EventFlag;
struct GDSEntry;
class Zone;
}
}

namespace Gui {
class IGuiManager;
}

namespace Game {

class EncounterHandler
{
public:
    using TransitionCallback = std::function<void(
        BAK::ZoneNumber,
        BAK::GamePositionAndHeading)>;

    EncounterHandler(
        BAK::GameState& gameState,
        Gui::IGuiManager& guiManager,
        Camera& camera);

    void SetTransitionCallback(TransitionCallback&& callback);

    void DoEncounter(const BAK::Encounter::Encounter& encounter);

    CombatEncounterHandler& GetCombatHandler();

private:
    void DoBlockEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Block& block);
    void DoDialogEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Dialog& dialog);
    void DoZoneEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Zone& zone);
    void DoGDSEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::GDSEntry& gds);
    void DoEventFlagEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::EventFlag& flag);

    BAK::GameState& mGameState;
    Gui::IGuiManager& mGuiManager;
    Camera& mCamera;
    Gui::DynamicDialogScene mDynamicDialogScene;
    CombatEncounterHandler mCombatHandler;
    glm::vec2 mSavedAngle{};
    TransitionCallback mTransitionCallback;
    const Logging::Logger& mLogger;
};

}