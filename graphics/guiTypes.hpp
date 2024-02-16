#pragma once

#include "graphics/types.hpp"

#include <glm/glm.hpp>

#include <ostream>

namespace Graphics {

enum class DrawMode
{
    Rect = 0,
    Sprite = 1,
    ClipRegion = 2,
};

enum class ColorMode
{
    // Use the color from the given texture
    Texture = 0,
    // Use a solid color
    SolidColor = 1,
    // Tint the texture this color (respects texture alpha)
    TintColor = 2 ,
    // Replace the textures color with this, respecting the texture alpha
    ReplaceColor = 3 
};


struct DrawInfo
{
    Graphics::DrawMode mDrawMode;
    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::TextureIndex mTexture;
    Graphics::ColorMode mColorMode;
    glm::vec4 mColor;
};

struct PositionInfo
{
    glm::vec2 mPosition;
    glm::vec2 mDimensions; // actually more like scale than dim.....
    float mRotation;
    bool mChildrenRelative;
};

std::ostream& operator<<(std::ostream& os, const ColorMode&);
std::ostream& operator<<(std::ostream& os, const DrawMode&);
std::ostream& operator<<(std::ostream& os, const DrawInfo&);
std::ostream& operator<<(std::ostream& os, const PositionInfo&);


}
