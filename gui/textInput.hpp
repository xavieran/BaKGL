#pragma once

#include "graphics/glfw.hpp"

#include "gui/core/widget.hpp"

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
        mTextBox{glm::vec2{}, dim},
        mText{},
        mMaxChars{maxChars},
        mHaveFocus{false}
    {
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
            SetColor(glm::vec4{0, 0, 0, 0});
        }
        else
        {
            SetColor(glm::vec4{0, 0, 0, .2});
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
        mTextBox.AddText(mFont, mText);
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
    TextBox mTextBox;
    std::string mText;
    unsigned mMaxChars;
    bool mHaveFocus;
};

}
