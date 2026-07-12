#include "game/interactable/catapult.hpp"

#include <cassert>

namespace Game::Interactable {

Catapult::Catapult(
    Gui::IGuiManager& guiManager,
    BAK::Target target,
    const EncounterCallback& encounterCallback,
    const CatapultCallback& catapultCallback)
:
    Generic(guiManager, target, encounterCallback),
    mCatapultCallback{catapultCallback}
{
}


void Catapult::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    assert(mCatapultCallback);
    std::invoke(mCatapultCallback);
}

}
