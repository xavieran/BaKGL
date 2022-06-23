#include "game/interactable/factory.hpp"

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

}
