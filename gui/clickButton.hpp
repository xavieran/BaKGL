#pragma once

#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include "com/assert.hpp"
#include "com/visit.hpp"

#include <variant>

namespace Gui {

class ClickButtonBase : public Widget
{
public:
    ClickButtonBase(
        glm::vec2 pos,
        glm::vec2 dims,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{0},
            true
        },
        mLeftPressedCallback{std::move(onLeftMousePress)},
        mRightPressedCallback{std::move(onRightMousePress)}
    {
        ASSERT(mLeftPressedCallback);
        ASSERT(mRightPressedCallback);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const RightMousePress& p){ return RightMousePressed(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            Logging::LogSpam("ClickButtonBase") << "Got LMC: " << this << " " << click << std::endl;
            std::invoke(mLeftPressedCallback);
            return true;
        }

        return false;
    }

    bool RightMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            Logging::LogSpam("ClickButtonBase") << "Got RMC: " << this << " " << click << std::endl;
            std::invoke(mRightPressedCallback);
            return true;
        }

        return false;
    }

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
        std::function<void()>&& onLeftMousePress)
    :
        ClickButtonBase{
            pos,
            dims,
            std::move(onLeftMousePress),
            [](){}
        },
        mFont{font},
        mNormal{
            glm::vec2{0, 0},
            dims,
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow},
        mPressed{
            glm::vec2{0, 0},
            dims,
            Color::buttonPressed,
            Color::buttonShadow,
            Color::buttonHighlight},
        mButtonPressed{false},
        mText{
            glm::vec2{3, 2},
            dims}
    {
        SetText(label);
        AddChildren();
    }

    void SetText(std::string_view label)
    {
        const auto textPos = glm::vec2{3, 2};
        const auto& dims = GetPositionInfo().mDimensions;
        const auto& [endPos, text] = mText.AddText(mFont, label);
        mText.SetPosition(
            glm::vec2{
                textPos.x + (dims.x - endPos.x) / 2,
                textPos.y});
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        const bool dirty = std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const LeftMouseRelease& p){ return LeftMouseReleased(p.mValue); },
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);

        const bool handled = ClickButtonBase::OnMouseEvent(event);

        if (dirty || std::holds_alternative<LeftMouseRelease>(event))
        {
            AddChildren();
        }

        return handled;
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            return UpdateState(true);
        }

        return false;
    }

    bool LeftMouseReleased(glm::vec2 click)
    {
        UpdateState(false);
        return false;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!Within(pos))
        {
            return UpdateState(false);
        }

        return false;
    }

private:
    void AddChildren()
    {
        ClearChildren();
        if (mButtonPressed)
        {
            AddChildBack(&mPressed);
        }
        else
        {
            AddChildBack(&mNormal);
        }

        AddChildBack(&mText);
    }

    bool UpdateState(bool newState)
    {
        const bool tmp = mButtonPressed;
        mButtonPressed = newState;
        return mButtonPressed != tmp;
    }

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
        std::function<void()>&& onRightMousePress)
    :
        ClickButtonBase{
            pos,
            dims,
            std::move(onLeftMousePress),
            std::move(onRightMousePress)
        },
        mNormal{
            Graphics::DrawMode::Sprite,
            spriteSheet,
            normal,
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0},
            dims,
            true},
        mPressed{
            Graphics::DrawMode::Sprite,
            spriteSheet,
            pressed,
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0},
            dims,
            true},
        mButton{}
    {
        mButton.emplace_back(&mNormal);
    }

    const std::vector<Graphics::IGuiElement*>& GetChildren() const override
    {
        return mButton;
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const LeftMouseRelease& p){ return LeftMouseReleased(p.mValue); },
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);

        return ClickButtonBase::OnMouseEvent(event);
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        ASSERT(mButton.size() >= 1);
        if (Within(click))
            mButton[0] = &mPressed;

        return false;
    }

    bool LeftMouseReleased(glm::vec2 click)
    {
        ASSERT(mButton.size() >= 1);
        mButton[0] = &mNormal;
        return false;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!Within(pos))
        {
            ASSERT(mButton.size() >= 1);
            mButton[0] = &mNormal;
        }

        return false;
    }

    void CenterImage(glm::vec2 dims)
    {
        // Set the image to its normal size and center it
        mNormal.SetDimensions(dims);
        mNormal.SetCenter(GetCenter() - GetTopLeft());
        mPressed.SetDimensions(dims);
        mPressed.SetCenter(GetCenter() - GetTopLeft());
    }

    void SetTexture(Graphics::SpriteSheetIndex ss, Graphics::TextureIndex ti)
    {
        mNormal.SetSpriteSheet(ss);
        mNormal.SetTexture(ti);
        mPressed.SetSpriteSheet(ss);
        mPressed.SetTexture(ti);
    }

private:
    Widget mNormal;
    Widget mPressed;
    
    std::vector<Graphics::IGuiElement*> mButton;
};

}
