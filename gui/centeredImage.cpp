#include "gui/centeredImage.hpp"

namespace Gui {

CenteredImage::CenteredImage(
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

void CenteredImage::SetImage(
    Graphics::SpriteSheetIndex spriteSheet,
    Graphics::TextureIndex textureIndex,
    glm::vec2 dims)
{
    mImage.SetSpriteSheet(spriteSheet);
    mImage.SetTexture(textureIndex);
    mImage.SetDimensions(dims);
    mImage.SetCenter(GetCenter() - GetTopLeft());
}

void CenteredImage::AddChildren()
{
    ClearChildren();
    AddChildBack(&mImage);
}

}
