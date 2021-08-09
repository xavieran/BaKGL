#pragma once

#include "graphics/glm.hpp"
#include "graphics/types.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <optional>
#include <vector>
#include <unordered_map>

namespace Graphics {

enum class DrawMode
{
    Rect = 0,
    Sprite = 1,
    ClipRegion = 2
};

enum class ColorMode
{
    // Use the color from the given texture
    Texture = 0,
    // Use a solid color
    SolidColor = 1,
    // Tint the texture this color (respects texture alpha)
    TintColor = 2 
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
    glm::vec3 mScale;

    bool mClipToDims;

    std::vector<Graphics::IGuiElement*> mChildren;
};

}
