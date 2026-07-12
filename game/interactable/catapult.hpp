#pragma once

#include "game/interactable/generic.hpp"

namespace Game::Interactable {

class Catapult : public Generic
{
public:
    Catapult(
        Gui::IGuiManager& guiManager,
        BAK::Target target,
        const EncounterCallback& encounterCallback,
        const CatapultCallback& catapultCallback);

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;

private:
    const CatapultCallback& mCatapultCallback;
};

}
