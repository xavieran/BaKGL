#pragma once

#include "gui/button.hpp"
#include "gui/core/widget.hpp"

#include "gui/textBox.hpp"

namespace Gui {
class Font;

class TextInput : public Widget
{
public:
    TextInput(
        const Font& font,
        glm::vec2 pos,
        glm::vec2 dim,
        unsigned maxChars);
    
    bool OnMouseEvent(const MouseEvent& event) override;
    bool OnKeyEvent(const KeyEvent& event) override;

    void SetText(const std::string& text);
    const std::string& GetText() const;
    void SetFocus(bool focus);
private:
    bool LeftMousePressed(const auto& clickPos);
    bool KeyPressed(int key);
    void RefreshText();
    bool CharacterEntered(char character);

    const Font& mFont;
    Button mButton;
    Widget mHighlight;
    TextBox mTextBox;
    std::string mText;
    unsigned mMaxChars;
    bool mHaveFocus;
};

}
