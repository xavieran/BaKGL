#pragma once

#include "game/interactable/IInteractable.hpp"

#include <memory>
#include <string_view>

namespace BAK {
class GameState;
}

namespace Gui {
class IGuiManager;
}

namespace Game {

enum class InteractableType 
{
    Chest       = 0,
    RiftMachine = 3,
    Building    = 4,
    Tombstone   = 6,
    Sign        = 7,
    Pit         = 9,
    Body        = 10,
    DirtMound   = 11,
    Corn        = 12,
    Campfire    = 13,
    Tunnel0     = 14,
    Door        = 17,
    CrystalTree = 18,
    Stones      = 19,
    FoodBush    = 20,
    PoisonBush  = 21,
    HealthBush  = 22,
    Slab        = 23,
    Stump       = 24,
    Well        = 25,
    SiegeEngine = 27,
    Scarecrow   = 28,
    DeadAnimal  = 29,
    Catapult    = 30,
    Column      = 31,
    Tunnel1     = 33,
    Bag         = 35,
    Ladder      = 36,
    DeadCombatant = 100,
    LivingCombatant = 101,
};

std::string_view ToString(InteractableType);

class InteractableFactory
{
public:
    InteractableFactory(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState,
        EncounterCallback&& encounterCallback);

    std::unique_ptr<IInteractable> MakeInteractable(
        BAK::EntityType entity) const;

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    EncounterCallback mEncounterCallback;
};

}
