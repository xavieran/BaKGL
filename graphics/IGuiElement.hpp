#pragma once

#include "graphics/glm.hpp"
#include "graphics/types.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <optional>
#include <ostream>
#include <vector>
#include <unordered_map>

namespace Graphics {

enum class DrawMode
{
    Rect = 0,
    Sprite = 1,
    ClipRegion = 2
};

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

enum class ColorMode
{
    // Use the color from the given texture
    Texture = 0,
    // Use a solid color
    SolidColor = 1,
    // Tint the texture this color (respects texture alpha)
    TintColor = 2 
};

std::ostream& operator<<(std::ostream& os, const ColorMode& dm)
{
    switch (dm)
    {
    case ColorMode::Texture:    return os << "Texture";
    case ColorMode::SolidColor: return os << "SolidColor";
    case ColorMode::TintColor:  return os << "TintColor";
    default:  return os << "Unknown";
    }
}

class SolidRect
{
    glm::vec4 mColor;
    glm::vec3 mPosition;
    glm::vec3 mDimensions;
};

class Sprite
{
    SpriteSheetIndex mSpriteSheet;
    TextureIndex mTexture;
    glm::vec3 mPosition;
    glm::vec3 mDimensions;
};

class ClipRegion
{
    glm::vec3 mPosition;
    glm::vec3 mDimensions;
};

class IGuiElement
{
public:
    IGuiElement(
        DrawMode drawMode,
        SpriteSheetIndex spriteSheet,
        TextureIndex texture,
        ColorMode colorMode,
        glm::vec4 color,
        glm::vec3 position,
        glm::vec3 dims,
        bool clipToDims)
    :
        mDrawMode{drawMode},
        mSpriteSheet{spriteSheet},
        mTexture{texture},
        mColorMode{colorMode},
        mColor{color},
        mPosition{position},
        mDimensions{dims},
        mClipToDims{clipToDims},
        mChildren{}
    {}

    void AddChildFront(Graphics::IGuiElement* elem)
    {
        mChildren.insert(mChildren.begin(), elem);
    }

    void AddChildBack(Graphics::IGuiElement* elem)
    {
        mChildren.emplace_back(elem);
    }


    virtual const std::vector<IGuiElement*>& GetChildren() const
    {
        return mChildren;
    }

    virtual ~IGuiElement(){}

    Graphics::DrawMode mDrawMode;
    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::TextureIndex mTexture;
    Graphics::ColorMode mColorMode;
    glm::vec4 mColor;
    
    glm::vec3 mPosition;
    glm::vec3 mDimensions;

    bool mClipToDims;

    std::vector<Graphics::IGuiElement*> mChildren;
};

std::ostream& operator<<(std::ostream& os, const IGuiElement& element)
{
    os  << "Element: { dm: " << element.mDrawMode << ", ss: " << element.mSpriteSheet
        << " , tex: " << element.mTexture << ", cm: " << element.mColorMode
        << " , pos: " << element.mPosition << " , dim: " << element.mDimensions
        << " , clip: " << element.mClipToDims << " , childs: " << element.mChildren.size()
        << " }";
    return os;
}

}
