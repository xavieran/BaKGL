#include "gui/textInput.hpp"

#include "com/visit.hpp"

#include <GLFW/glfw3.h>

namespace Gui {

TextInput::TextInput(
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

bool TextInput::OnMouseEvent(const MouseEvent& event)
{
    return std::visit(overloaded{
        [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
        [](const auto&){ return false; }
        }, event);
}

bool TextInput::OnKeyEvent(const KeyEvent& event)
{
    return std::visit(overloaded{
        [this](const KeyPress& p){ return KeyPressed(p.mValue); },
        [this](const Character& p){ return CharacterEntered(p.mValue); },
        [](const auto&){ return false; }
        }, event);
}

void TextInput::SetText(const std::string& text)
{
    mText = text;
    RefreshText();
}

const std::string& TextInput::GetText() const
{
    return mText;
}

void TextInput::SetFocus(bool focus)
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

bool TextInput::LeftMousePressed(const auto& clickPos)
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

bool TextInput::KeyPressed(int key)
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

void TextInput::RefreshText()
{
    const auto [pos, _] = mTextBox.SetText(mFont, mText);
    mHighlight.SetPosition(glm::vec2{pos.x - 2, 2});
    
}

bool TextInput::CharacterEntered(char character)
{
    if (mHaveFocus && mText.size() < mMaxChars)
    {
        mText += character;
        RefreshText();
        return true;
    }

    return false;
}

}
