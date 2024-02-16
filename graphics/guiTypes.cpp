#include "graphics/guiTypes.hpp"

#include "graphics/glm.hpp"

namespace Graphics {

std::ostream& operator<<(std::ostream& os, const ColorMode& dm)
{
    switch (dm)
    {
    case ColorMode::Texture:    return os << "Texture";
    case ColorMode::SolidColor: return os << "SolidColor";
    case ColorMode::TintColor:  return os << "TintColor";
    case ColorMode::ReplaceColor:  return os << "ReplaceColor";
    default:  return os << "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const DrawMode& dm)
{
    switch (dm)
    {
    case DrawMode::Rect:       return os << "Rect";
    case DrawMode::Sprite:     return os << "Sprite";
    case DrawMode::ClipRegion: return os << "ClipRegion";
    default:  return os << "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const DrawInfo& di)
{
    os  << "DrawInfo={ dm: " << di.mDrawMode << ", ss: " << di.mSpriteSheet
        << " , tex: " << di.mTexture << ", cm: " << di.mColorMode << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PositionInfo& pi)
{
    os  << "PositionInfo={ pos: " << pi.mPosition
        << ", scale: " << pi.mDimensions
        << ", rotation: " << glm::degrees(pi.mRotation)
        << " , childRel: " << pi.mChildrenRelative
        << "}";
    return os;
}


}
