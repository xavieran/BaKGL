#pragma once

#include "graphics/sprites.hpp"

#include <glm/glm.hpp>

namespace Gui {

class Backgrounds
{
public:

    Backgrounds(
        Graphics::SpriteManager& spriteManager);

    Graphics::SpriteSheetIndex GetSpriteSheet() const;
    Graphics::TextureIndex GetScreen(const std::string& scx) const;

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::unordered_map<std::string, Graphics::TextureIndex> mScxToSprite;

    const Logging::Logger& mLogger;
};

}
