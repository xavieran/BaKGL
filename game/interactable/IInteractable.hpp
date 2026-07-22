#pragma once

#include "bak/entityType.hpp"
#include "bak/types.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace BAK {
class GenericContainer;
}

namespace Game {

using EncounterCallback = std::function<bool(glm::uvec2)>;
using DoorStateCallback = std::function<void(BAK::DoorIndex doorIndex, bool isOpen)>;
using CatapultCallback = std::function<void()>;
using PitCrossCallback = std::function<void(BAK::EntityIndex)>;


class IInteractable
{
public:
    virtual void BeginInteraction(BAK::GenericContainer&, BAK::EntityType, BAK::EntityIndex) = 0;
    virtual void EncounterFinished() = 0;

    virtual ~IInteractable() {}
};

}
