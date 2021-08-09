#pragma once

#include "graphics/glm.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/types.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <unordered_map>

namespace Gui {

class IGuiElement
{
public:

    virtual const std::vector<IGuiElement*>& GetChildren() const = 0;
    virtual ~IGuiElement() = 0;

    std::optional<Graphics::SpriteSheetIndex> mSpriteSheet;
    Graphics::TextureIndex mTexture;
    Graphics::ColorMode mColorMode;
    glm::vec4 mColor;
    
    glm::vec3 mPosition;
    glm::vec3 mDims;
    glm::vec3 mScale;

    bool mClipToDims;
};

}
