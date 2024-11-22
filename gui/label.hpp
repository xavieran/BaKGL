#pragma once

#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class Font;

class Label : public Widget
{
public:

    Label(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& fr,
        const std::string& text);
    
    void SetText(std::string_view text);

    Widget mForeground;
    TextBox mTextBox;
    const Font& mFont;
};

}
