#pragma once

#include "gui/button.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class SplitStackDialog
    : public Widget
{
public:
    SplitStackDialog(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        std::function<void(bool, unsigned)>)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{1.0, 0, 0, .3},
            true},
        mFont{font},
        mFrame{
            glm::vec2{0},
            dims,
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow
        },
        mAmount{
            glm::vec2{0},
            dims
        },
        mLogger{Logging::LogState::GetLogger("Gui::SplitStackDialog")}
    {
        AddChildBack(&mFrame);
        AddChildBack(&mAmount);
    }

private:
    const Font& mFont;

    Button mFrame;

    TextBox mAmount;
    //ClickButton mIncrease;
    //ClickButton mDecrease;
    //ClickButton mGive;
    //ClickButton mShare;

    const Logging::Logger& mLogger;
};

}
