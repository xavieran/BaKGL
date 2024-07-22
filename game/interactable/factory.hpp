#pragma once

#include "game/interactable/IInteractable.hpp"
#include "game/interactable/building.hpp"
#include "game/interactable/chest.hpp"
#include "game/interactable/combatant.hpp"
#include "game/interactable/door.hpp"
#include "game/interactable/generic.hpp"
#include "game/interactable/ladder.hpp"
#include "game/interactable/pit.hpp"
#include "game/interactable/tomb.hpp"

#include "gui/IGuiManager.hpp"

#include "bak/gameState.hpp"

#include <memory>

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
        EncounterCallback&& encounterCallback)
    :
        mGuiManager{guiManager},
        mGameState{gameState},
        mEncounterCallback{std::move(encounterCallback)}
    {}

    std::unique_ptr<IInteractable> MakeInteractable(
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

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    EncounterCallback mEncounterCallback;
};

}
