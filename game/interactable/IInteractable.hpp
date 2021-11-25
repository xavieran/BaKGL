#pragma once

#include "bak/container.hpp"

namespace Game {

using EncounterCallback = std::function<void(glm::uvec2)>;

class IInteractable
{
public:
    virtual void BeginInteraction(BAK::GenericContainer&) = 0;
    virtual void EncounterFinished() = 0;

    virtual ~IInteractable() {};
};

}
