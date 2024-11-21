#pragma once

#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class Font;

class TextBox : public Widget
{
public:
    TextBox(
        glm::vec2 pos,
        glm::vec2 dim);
    
    std::pair<glm::vec2, std::string_view> SetText(
        const Font& fr,
        std::string_view text,
        bool centerHorizontal=false,
        bool centerVertical=false,
        bool isBold=false,
        double newLineMultiplier=1.0,
        float scale=1.0);

private:
    std::vector<Widget> mText;
};

}
