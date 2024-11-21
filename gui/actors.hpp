#pragma once

#include "graphics/types.hpp"

#include <glm/glm.hpp>

namespace Logging {
class Logger;
}

namespace Graphics {
class SpriteManager;
}

namespace Gui {

class Actors
{
public:

    Actors(
        Graphics::SpriteManager& spriteManager);

    Graphics::SpriteSheetIndex GetSpriteSheet() const;

    std::pair<
        Graphics::TextureIndex,
        glm::vec2>
    GetActor(unsigned actor) const;

    std::pair<
        Graphics::TextureIndex,
        glm::vec2>
    GetActorA(unsigned actor) const;

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::vector<std::pair<Graphics::TextureIndex, glm::vec2>> mActorDimensions;
    std::unordered_map<
        unsigned,
        std::pair<Graphics::TextureIndex, glm::vec2>> mActorADimensions;

    const Logging::Logger& mLogger;
};

}
