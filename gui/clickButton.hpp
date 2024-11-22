#pragma once

#include "gui/button.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"
#include "gui/core/mouseEvent.hpp"

namespace Gui {

class Font;

class ClickButtonBase : public Widget
{
public:
    ClickButtonBase(
        glm::vec2 pos,
        glm::vec2 dims,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress);

    bool OnMouseEvent(const MouseEvent& event) override;
    bool LeftMousePressed(glm::vec2 click);
    bool RightMousePressed(glm::vec2 click);

private:
    std::function<void()> mLeftPressedCallback;
    std::function<void()> mRightPressedCallback;
};


class ClickButton : public ClickButtonBase
{
public:
    ClickButton(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const std::string& label,
        std::function<void()>&& onLeftMousePress);
    
    void SetText(std::string_view label);
    bool OnMouseEvent(const MouseEvent& event) override;
    bool LeftMousePressed(glm::vec2 click);
    bool LeftMouseReleased(glm::vec2 click);
    bool MouseMoved(glm::vec2 pos);

private:
    void AddChildren();
    bool UpdateState(bool newState);

    const Font& mFont;

    Button mNormal;
    Button mPressed;
    bool mButtonPressed;
    TextBox mText;
};

class ClickButtonImage : public ClickButtonBase
{
public:
    ClickButtonImage(
        glm::vec2 pos,
        glm::vec2 dims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex normal,
        Graphics::TextureIndex pressed,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress);
    
    bool OnMouseEvent(const MouseEvent& event) override;
    bool LeftMousePressed(glm::vec2 click);
    bool LeftMouseReleased(glm::vec2 click);
    bool MouseMoved(glm::vec2 pos);
    void CenterImage(glm::vec2 dims);
    void SetTexture(Graphics::SpriteSheetIndex ss, Graphics::TextureIndex ti);
    void SetColor(glm::vec4 color);
    void SetColorMode(Graphics::ColorMode mode);
private:
    void AddChildren();

    bool mIsPressed;
    Widget mNormal;
    Widget mPressed;
};

}
