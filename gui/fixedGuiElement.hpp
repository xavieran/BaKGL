#pragma once

#include "graphics/IGuiElement.hpp"
#include "graphics/guiTypes.hpp"

namespace Gui {

class FixedGuiElement : public Graphics::IGuiElement
{
public:
    FixedGuiElement(
        Graphics::DrawMode drawMode,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex texture,
        Graphics::ColorMode colorMode,
        glm::vec4 color,
        glm::vec3 pos,
        glm::vec3 dims,
        bool childrenRelative)
    :
        mDrawInfo{
            drawMode,
            spriteSheet,
            texture,
            colorMode,
            color},
        mPositionInfo{
            pos,
            dims,
            childrenRelative}
    {}

    const Graphics::DrawInfo& GetDrawInfo() const override
    {
        return mDrawInfo;
    }

    const Graphics::PositionInfo& GetPositionInfo() const override
    {
        return mPositionInfo;
    }

protected:
    Graphics::DrawInfo mDrawInfo;
    Graphics::PositionInfo mPositionInfo;
};

}
