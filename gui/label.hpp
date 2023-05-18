#pragma once

#include "gui/colors.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class Label : public Widget
{
public:

    Label(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& fr,
        const std::string& text)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            Color::frameMaroon,
            true
        },
        mForeground{
            RectTag{},
            glm::vec2{1,1},
            dims,
            Color::buttonBackground,
            true
        },
        mTextBox{
            glm::vec2{3, 2},
            dims
        },
        mFont{fr},
        mLogger{Logging::LogState::GetLogger("Gui::Label")}
    {
        AddChildBack(&mForeground);
        AddChildBack(&mTextBox);
        SetText(text);
    }

    void SetText(std::string_view text)
    {
        auto [dims, remaining] = mTextBox
            .AddText(mFont, text);
        // Add margin
        dims += glm::vec2{3, 4};
        // Resize to flow around text
        SetDimensions(dims);
        mForeground.SetDimensions(
            dims - glm::vec2{2,2});
    }

    Widget mForeground;
    TextBox mTextBox;
    const Font& mFont;
    const Logging::Logger& mLogger;
};

}
