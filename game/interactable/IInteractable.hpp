#pragma once

#include "bak/entityType.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <string_view>

namespace BAK {
class GenericContainer;
}

namespace Game {

using EncounterCallback = std::function<void(glm::uvec2)>;

class IInteractable
{
public:
    virtual void BeginInteraction(BAK::GenericContainer&, BAK::EntityType) = 0;
    virtual void EncounterFinished() = 0;

    virtual ~IInteractable() {}
};

}
