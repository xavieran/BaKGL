#pragma once

#include "gui/core/widget.hpp"

namespace Gui {

class CenteredImage : public Widget
{
public:
    CenteredImage(
        glm::vec2 pos,
        glm::vec2 dims)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            //glm::vec4{1,0,0,.3},
            glm::vec4{},
            true
        },
        mImage{
            ImageTag{},
            Graphics::SpriteSheetIndex{0},
            Graphics::TextureIndex{0},
            pos,
            glm::vec2{},
            true
        }
    {
        AddChildren();
    }

    void SetImage(
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex textureIndex,
        glm::vec2 dims)
    {
        mImage.SetSpriteSheet(spriteSheet);
        mImage.SetTexture(textureIndex);
        mImage.SetDimensions(dims);
        mImage.SetCenter(GetCenter() - GetTopLeft());
    }

private:
    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mImage);
    }

    Widget mImage;
};
 
}
