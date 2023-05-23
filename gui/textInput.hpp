#pragma once

#include "graphics/glfw.hpp"

#include "gui/button.hpp"
#include "gui/core/widget.hpp"
#include "gui/colors.hpp"

#include "gui/fontManager.hpp"
#include "gui/textBox.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

namespace Gui {

class TextInput : public Widget
{
public:
    TextInput(
        const Font& font,
        glm::vec2 pos,
        glm::vec2 dim,
        unsigned maxChars)
    :
        Widget{
            RectTag{},
            pos,
            dim,
            glm::vec4{0},
            true
        },
        mFont{font},
        mButton{glm::vec2{}, dim, Color::buttonBackground, Color::buttonShadow, Color::buttonShadow},
        mHighlight{RectTag{}, glm::vec2{2}, glm::vec2{3, dim.y - 4}, {}, true},
        mTextBox{glm::vec2{2}, dim - glm::vec2{2}},
        mText{},
        mMaxChars{maxChars},
        mHaveFocus{false}
    {
        AddChildBack(&mButton);
        AddChildBack(&mHighlight);
        AddChildBack(&mTextBox);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [](const auto&){ return false; }
            }, event);
    }

    bool OnKeyEvent(const KeyEvent& event) override
    {
        return std::visit(overloaded{
            [this](const KeyPress& p){ return KeyPressed(p.mValue); },
            [this](const Character& p){ return CharacterEntered(p.mValue); },
            [](const auto&){ return false; }
            }, event);
    }

    void SetText(const std::string& text)
    {
        mText = text;
        RefreshText();
    }

    const std::string& GetText() const
    {
        return mText;
    }

    void SetFocus(bool focus)
    {
        mHaveFocus = focus;
        if (!mHaveFocus)
        {
            mHighlight.SetColor(glm::vec4{0, 0, 0, 0});
        }
        else
        {
            mHighlight.SetColor(glm::vec4{0, 0, 0, .2});
        }
    }

private:
    bool LeftMousePressed(const auto& clickPos)
    {
        if (Within(clickPos))
        {
            SetFocus(true);
        }
        else
        {
            SetFocus(false);
        }
        return false;
    }

    bool KeyPressed(int key)
    {
        if (mHaveFocus)
        {
            if (key == GLFW_KEY_BACKSPACE)
            {
                if (mText.size() > 0)
                {
                    mText.pop_back();
                    RefreshText();
                }
                return true;
            }
        }
        return false;
    }

    void RefreshText()
    {
        const auto [pos, _] = mTextBox.AddText(mFont, mText);
        mHighlight.SetPosition(glm::vec2{pos.x - 2, 2});
        
    }

    bool CharacterEntered(char character)
    {
        if (mHaveFocus && mText.size() < mMaxChars)
        {
            mText += character;
            RefreshText();
            return true;
        }

        return false;
    }

    const Font& mFont;
    Button mButton;
    Widget mHighlight;
    TextBox mTextBox;
    std::string mText;
    unsigned mMaxChars;
    bool mHaveFocus;
};

}
