#pragma once

#include "graphics/sprites.hpp"

namespace Gui {

struct ButtonTextures
{
    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::TextureIndex mNormal;
    Graphics::TextureIndex mPressed;
};

}
