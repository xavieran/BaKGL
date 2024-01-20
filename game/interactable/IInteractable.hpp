#pragma once

#include "bak/container.hpp"
#include "bak/entityType.hpp"

namespace Game {

using EncounterCallback = std::function<void(glm::uvec2)>;

class IInteractable
{
public:
    virtual void BeginInteraction(BAK::GenericContainer&, BAK::EntityType) = 0;
    virtual void EncounterFinished() = 0;

    virtual ~IInteractable() {};
};

}
