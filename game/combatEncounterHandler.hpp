#pragma once

#include "bak/types.hpp"
#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace BAK {
class GameState;
namespace Encounter {
class Encounter;
class Combat;
}
}

namespace Gui {
class IGuiManager;
class DynamicDialogScene;
}

namespace Game {

struct CombatCheckResult
{
    bool mCombatActive;
    bool mCombatScouted;
};

class CombatEncounterHandler
{
public:
    CombatEncounterHandler(
        BAK::GameState& gameState,
        Gui::IGuiManager& guiManager,
        Gui::DynamicDialogScene& dynamicDialogScene);

    CombatCheckResult CheckCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);

    void CheckAndDoCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);

private:
    BAK::GameState& mGameState;
    Gui::IGuiManager& mGuiManager;
    Gui::DynamicDialogScene& mDynamicDialogScene;
    const Logging::Logger& mLogger;
};

}