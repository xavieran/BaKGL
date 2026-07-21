#include "game/interactable/factory.hpp"

#include "bak/gameState.hpp"
#include "bak/dialogSources.hpp"

#include "game/interactable/building.hpp"
#include "game/interactable/catapult.hpp"
#include "game/interactable/chest.hpp"
#include "game/interactable/combatant.hpp"
#include "game/interactable/corpse.hpp"
#include "game/interactable/door.hpp"
#include "game/interactable/description.hpp"
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
    EncounterCallback&& encounterCallback,
    DoorStateCallback&& doorStateCallback,
    CatapultCallback&& catapultCallback)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mEncounterCallback{std::move(encounterCallback)},
    mDoorStateCallback{std::move(doorStateCallback)},
    mCatapultCallback{std::move(catapultCallback)}
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
    case InteractableType::Catapult:
        return std::make_unique<Catapult>(
            mGuiManager,
            BAK::KeyTarget{0},
            mEncounterCallback,
            mCatapultCallback);
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
            mGameState,
            mDoorStateCallback);
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
    case InteractableType::RiftMachine:
        return MakeGeneric(BAK::DialogSources::mUnknownObject);
    case InteractableType::DeadCombatant:
        return std::make_unique<Corpse>(
            mGuiManager,
            BAK::KeyTarget{0});
    case InteractableType::LivingCombatant:
        return std::make_unique<Combatant>(
            mGuiManager,
            mGameState);
    default:
        Logging::LogFatal(__FUNCTION__) << "Unhandled entity type: " 
            << static_cast<unsigned>(entity) << " interactableType: "
            << ToString(interactableType) << std::endl;
        return MakeGeneric(BAK::DialogSources::mUnknownObject);
    }
}

std::unique_ptr<IInteractable> InteractableFactory::MakeRightClickInteractable(
    BAK::EntityType entity,
    BAK::GenericContainer& container) const
{
    using namespace Interactable;
    constexpr auto nonInteractables = 6;
    const auto interactableType = static_cast<InteractableType>(
        static_cast<unsigned>(entity) - nonInteractables);

    Logging::LogDebug(__FUNCTION__) << " Handling: " << ToString(interactableType) << "\n";

    BAK::Target dialog;

    const auto SetMonsterFromContainer = [&](const BAK::GenericContainer& c){
        if (c.GetHeader().HasCombat())
        {
            auto combatantIndex = mGameState.GetCombatantIndex(
                BAK::CombatRelInfo{
                    c.GetHeader().GetCombatNumber(),
                    c.GetHeader().GetCombatantNumber()});
            if (combatantIndex)
            {
                const auto& cgl = mGameState.GetCombatantGridLocation(*combatantIndex);
                mGameState.SetMonster(cgl.mMonster);
            }
        }
    };

    switch (interactableType)
    {
    case InteractableType::Bag:           dialog = BAK::DialogSources::mBagDescription; break;
    case InteractableType::Body:          dialog = BAK::DialogSources::mBodyDescription; break;
    case InteractableType::Campfire:      dialog = BAK::DialogSources::mCampfireDescription; break;
    case InteractableType::Corn:          dialog = BAK::DialogSources::mCornDescription; break;
    case InteractableType::CrystalTree:   dialog = BAK::DialogSources::mCrystalTreeDescription; break;
    case InteractableType::DirtMound:     dialog = BAK::DialogSources::mDirtpileDescription; break;
    case InteractableType::Stones:        dialog = BAK::DialogSources::mStonesDescription; break;
    case InteractableType::Scarecrow:     dialog = BAK::DialogSources::mScarecrowDescription; break;
    case InteractableType::SiegeEngine:   dialog = BAK::DialogSources::mSiegeEngineDescription; break;
    case InteractableType::Stump:         dialog = BAK::DialogSources::mStumpDescription; break;
    case InteractableType::DeadAnimal:    dialog = BAK::DialogSources::mTrappedAnimalDescription; break;
    case InteractableType::HealthBush:    dialog = BAK::DialogSources::mHealthBushDescription; break;
    case InteractableType::PoisonBush:    dialog = BAK::DialogSources::mPoisonBushDescription; break;
    case InteractableType::FoodBush:      dialog = BAK::DialogSources::mFoodBushDescription; break;
    case InteractableType::Well:          dialog = BAK::DialogSources::mWellDescription; break;
    case InteractableType::Column:        dialog = BAK::DialogSources::mColumnDescription; break;
    case InteractableType::Sign:          dialog = BAK::DialogSources::mSignDescription; break;
    case InteractableType::Slab:          dialog = BAK::DialogSources::mSlabDescription; break;
    case InteractableType::Tunnel0:       dialog = BAK::DialogSources::mTunnelDescription; break;
    case InteractableType::Tunnel1:       dialog = BAK::DialogSources::mTunnelDescription; break;
    case InteractableType::RiftMachine:   dialog = BAK::DialogSources::mRifMachineDescription; break;
    case InteractableType::Catapult:      dialog = BAK::DialogSources::mCatapultDescription; break;
    case InteractableType::Tombstone:     dialog = BAK::DialogSources::mTombstoneDescription; break;
    case InteractableType::Ladder:        dialog = BAK::DialogSources::mLadderDescription; break;
    case InteractableType::Pit:           dialog = BAK::DialogSources::mPitDescription; break;
    case InteractableType::Door:          dialog = BAK::DialogSources::mDoorDescription; break;
    case InteractableType::DeadCombatant:
        SetMonsterFromContainer(container);
        dialog = BAK::DialogSources::mDeadEnemyDescription;
        break;
    case InteractableType::LivingCombatant:
        SetMonsterFromContainer(container);
        dialog = BAK::DialogSources::mCombatantDescription;
        break;

    case InteractableType::Chest:
        if (!container.HasLock())
            dialog = BAK::DialogSources::mBoxChestCase3;
        else if (container.GetLock().IsFairyChest())
            dialog = BAK::DialogSources::mChestWithSpecialLockDescription;
        else
            dialog = BAK::DialogSources::mUnlockedChestCase0_2;
        break;

    case InteractableType::Building:
        if (container.HasDialog())
            mGameState.SetDialogContext_7530(container.GetDialog().mContextVar);
        dialog = BAK::DialogSources::mBuildingDescription;
        break;

    default:
        dialog = BAK::DialogSources::mUnknownObject;
        break;
    }

    return std::make_unique<Description>(mGuiManager, dialog);
}

}
