#pragma once

#include "gui/colors.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

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
            Graphics::DrawMode::Rect,
            0,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::SolidColor,
            Color::frameMaroon,
            pos,
            dims,
            true
        },
        mForeground{
            Graphics::DrawMode::Rect,
            0,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::SolidColor,
            Color::buttonBackground,
            glm::vec2{1,1},
            dims,
            true
        },
        mTextBox{
            glm::vec2{3},
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
        dims += glm::vec2{3};
        mForeground.SetDimensions(
            dims - glm::vec2{2,2});

        mPositionInfo.mDimensions = dims;
    }

    Widget mForeground;
    TextBox mTextBox;
    const Font& mFont;
    const Logging::Logger& mLogger;
};

}
