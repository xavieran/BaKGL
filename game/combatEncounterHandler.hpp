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

    bool CombatIsUnavoidable(BAK::CombatIndex);

    CombatCheckResult CheckCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);

    unsigned CalculateAvoidanceStealth(
        const BAK::Encounter::Combat& combat);

    bool CheckAvoidCombatDueToStealth(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);

    bool CheckAndDoCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);

    void SetEnterCombatCallback(std::function<void()>&& callback);

    void UpdatePostEncounterFlags(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat);

private:
    BAK::GameState& mGameState;
    Gui::IGuiManager& mGuiManager;
    Gui::DynamicDialogScene& mDynamicDialogScene;
    std::function<void()> mEnterCombatCallback;
    const Logging::Logger& mLogger;
};

}
