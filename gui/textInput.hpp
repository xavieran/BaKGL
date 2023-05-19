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
            //glm::vec4{0},
            glm::vec4{0,1,0,.3},
            true
        },
        mFont{font},
        mTextBox{glm::vec2{}, dim},
        mString{},
        mMaxChars{maxChars},
        mHaveFocus{true}
    {
        AddChildBack(&mTextBox);
    }

    bool OnKeyEvent(const KeyEvent& event)
    {
        return std::visit(overloaded{
            [this](const KeyPress& p){ return KeyPressed(p.mValue); },
            [this](const Character& p){ return CharacterEntered(p.mValue); },
            [](const auto&){ return false; }
            }, event);
    }

private:
    bool KeyPressed(int key)
    {
        if (key == GLFW_KEY_BACKSPACE)
        {
            if (mString.size() > 0)
            {
                mString.pop_back();
                mTextBox.AddText(mFont, mString);
            }
            return true;
        }
        return false;
    }

    bool CharacterEntered(char character)
    {
        mString += character;
        mTextBox.AddText(mFont, mString);
        return true;
    }

    const Font& mFont;
    TextBox mTextBox;
    std::string mString;
    unsigned mMaxChars;
    bool mHaveFocus;
};

}
