#pragma once

#include "game/interactable/IInteractable.hpp"
#include "game/interactable/chest.hpp"
#include "game/interactable/generic.hpp"
#include "game/interactable/ladder.hpp"
#include "game/interactable/tomb.hpp"

#include "gui/IGuiManager.hpp"

#include "bak/gameState.hpp"
#include "bak/worldFactory.hpp"

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
};


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

        switch (interactableType)
        {
        case InteractableType::Bag:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mBag,
                mEncounterCallback);
        case InteractableType::Body:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mBody,
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
        case InteractableType::Campfire:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mCampfire,
                mEncounterCallback);
        case InteractableType::Corn:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mCorn,
                mEncounterCallback);
        case InteractableType::CrystalTree:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mCrystalTree,
                mEncounterCallback);
        case InteractableType::DirtMound:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mDirtpile,
                mEncounterCallback);
        case InteractableType::Stones:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mStones,
                mEncounterCallback);
        case InteractableType::Scarecrow:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mScarecrow,
                mEncounterCallback);
        case InteractableType::Stump:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mStump,
                mEncounterCallback);
        case InteractableType::SiegeEngine:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mSiegeEngine,
                mEncounterCallback);
        case InteractableType::DeadAnimal:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mTrappedAnimal,
                mEncounterCallback);
        case InteractableType::HealthBush:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mHealthBush,
                mEncounterCallback);
        case InteractableType::PoisonBush:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mPoisonBush,
                mEncounterCallback);
        case InteractableType::FoodBush:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mFoodBush,
                mEncounterCallback);
        case InteractableType::Well:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mWell,
                mEncounterCallback);
        case InteractableType::Column:  [[ fallthrough ]];
        case InteractableType::Sign:    [[ fallthrough ]];
        case InteractableType::Slab:    [[ fallthrough ]];
        case InteractableType::Tunnel0: [[ fallthrough ]];
        case InteractableType::Tunnel1:
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mUnknownObject,
                mEncounterCallback);
        default:
            Logging::LogFatal(__FUNCTION__) << "Unhandled entity type: " 
                << static_cast<unsigned>(entity) << std::endl;
            ASSERT(false);
            return std::make_unique<Generic>(
                mGuiManager,
                BAK::DialogSources::mUnknownObject,
                mEncounterCallback);
        }
    }

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    EncounterCallback mEncounterCallback;
};

}
