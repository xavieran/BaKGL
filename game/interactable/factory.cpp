#include "game/interactable/factory.hpp"

#include "bak/gameState.hpp"
#include "bak/dialogSources.hpp"

#include "game/interactable/building.hpp"
#include "game/interactable/chest.hpp"
#include "game/interactable/combatant.hpp"
#include "game/interactable/door.hpp"
#include "game/interactable/generic.hpp"
#include "game/interactable/ladder.hpp"
#include "game/interactable/pit.hpp"
#include "game/interactable/tomb.hpp"

#include "gui/IGuiManager.hpp"

namespace Game {

std::string_view ToString(InteractableType it)
{
    switch (it)
    {
        case InteractableType::Chest: return "Chest";
        case InteractableType::RiftMachine: return "RiftMachine";
        case InteractableType::Building: return "Building";
        case InteractableType::Tombstone: return "Tombstone";
        case InteractableType::Sign: return "Sign";
        case InteractableType::Pit: return "Pit";
        case InteractableType::Body: return "Body";
        case InteractableType::DirtMound: return "DirtMound";
        case InteractableType::Corn: return "Corn";
        case InteractableType::Campfire: return "Campfire";
        case InteractableType::Tunnel0: return "Tunnel0";
        case InteractableType::Door: return "Door";
        case InteractableType::CrystalTree: return "CrystalTree";
        case InteractableType::Stones: return "Stones";
        case InteractableType::FoodBush: return "FoodBush";
        case InteractableType::PoisonBush: return "PoisonBush";
        case InteractableType::HealthBush: return "HealthBush";
        case InteractableType::Slab: return "Slab";
        case InteractableType::Stump: return "Stump";
        case InteractableType::Well: return "Well";
        case InteractableType::SiegeEngine: return "SiegeEngine";
        case InteractableType::Scarecrow: return "Scarecrow";
        case InteractableType::DeadAnimal: return "DeadAnimal";
        case InteractableType::Catapult: return "Catapult";
        case InteractableType::Column: return "Column";
        case InteractableType::Tunnel1: return "Tunnel1";
        case InteractableType::Bag: return "Bag";
        case InteractableType::Ladder: return "Ladder";
        default: return "UnknownInteractable";
    }
}

InteractableFactory::InteractableFactory(
    Gui::IGuiManager& guiManager,
    BAK::GameState& gameState,
    EncounterCallback&& encounterCallback)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mEncounterCallback{std::move(encounterCallback)}
{}

std::unique_ptr<IInteractable> InteractableFactory::MakeInteractable(
    BAK::EntityType entity) const
{
    using namespace Interactable;
    constexpr auto nonInteractables = 6;
    const auto interactableType = static_cast<InteractableType>(
            static_cast<unsigned>(entity) - nonInteractables);

    Logging::LogDebug(__FUNCTION__) << " Handling: " << ToString(interactableType) << "\n";

    const auto MakeGeneric = [this](BAK::Target dialog){
        return std::make_unique<Generic>(
            mGuiManager,
            dialog,
            mEncounterCallback);
    };

    switch (interactableType)
    {
    case InteractableType::Building:
        return std::make_unique<Building>(
            mGuiManager,
            mGameState,
            mEncounterCallback);
    case InteractableType::Chest:
        return std::make_unique<Chest>(
            mGuiManager,
            mGameState);
    case InteractableType::Tombstone:
        return std::make_unique<Tomb>(
            mGuiManager,
            mGameState,
            mEncounterCallback);
    case InteractableType::Ladder:
        return std::make_unique<Ladder>(
            mGuiManager,
            mGameState);
    case InteractableType::Bag:
        return MakeGeneric(BAK::DialogSources::mBag);
    case InteractableType::Body:
        return MakeGeneric(BAK::DialogSources::mBody);
    case InteractableType::Campfire:
        return MakeGeneric(BAK::DialogSources::mCampfire);
    case InteractableType::Door:
        return std::make_unique<Door>(
            mGuiManager,
            mGameState);
    case InteractableType::Pit:
        return std::make_unique<Pit>(
            mGuiManager,
            mGameState);
    case InteractableType::Corn:
        return MakeGeneric(BAK::DialogSources::mCorn);
    case InteractableType::CrystalTree:
        return MakeGeneric(BAK::DialogSources::mCrystalTree);
    case InteractableType::DirtMound:
        return MakeGeneric(BAK::DialogSources::mDirtpile);
    case InteractableType::Stones:
        return MakeGeneric(BAK::DialogSources::mStones);
    case InteractableType::Scarecrow:
        return MakeGeneric(BAK::DialogSources::mScarecrow);
    case InteractableType::Stump:
        return MakeGeneric(BAK::DialogSources::mStump);
    case InteractableType::SiegeEngine:
        return MakeGeneric(BAK::DialogSources::mSiegeEngine);
    case InteractableType::DeadAnimal:
        return MakeGeneric(BAK::DialogSources::mTrappedAnimal);
    case InteractableType::HealthBush:
        return MakeGeneric(BAK::DialogSources::mHealthBush);
    case InteractableType::PoisonBush:
        return MakeGeneric(BAK::DialogSources::mPoisonBush);
    case InteractableType::FoodBush:
        return MakeGeneric(BAK::DialogSources::mFoodBush);
    case InteractableType::Well:
        return MakeGeneric(BAK::DialogSources::mWell);
    case InteractableType::Column:  [[ fallthrough ]];
    case InteractableType::Sign:    [[ fallthrough ]];
    case InteractableType::Slab:    [[ fallthrough ]];
    case InteractableType::Tunnel0: [[ fallthrough ]];
    case InteractableType::Tunnel1:
        return MakeGeneric(BAK::DialogSources::mUnknownObject);
    case InteractableType::DeadCombatant:
        return std::make_unique<Combatant>(
            mGuiManager,
            BAK::KeyTarget{0});
    default:
        Logging::LogFatal(__FUNCTION__) << "Unhandled entity type: " 
            << static_cast<unsigned>(entity) << " interactableType: "
            << ToString(interactableType) << std::endl;
        return MakeGeneric(BAK::DialogSources::mUnknownObject);
    }
}

}
