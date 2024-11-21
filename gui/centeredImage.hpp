#pragma once

#include "gui/core/widget.hpp"

namespace Gui {

class CenteredImage : public Widget
{
public:
    CenteredImage(
        glm::vec2 pos,
        glm::vec2 dims);
    
    void SetImage(
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex textureIndex,
        glm::vec2 dims);

private:
    void AddChildren();

    Widget mImage;
};
 
}
