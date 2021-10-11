#pragma once

#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/mouseEvent.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include "com/visit.hpp"

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
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            pos,
            dims,
            true
        },
        mLeftPressedCallback{std::move(onLeftMousePress)},
        mRightPressedCallback{std::move(onRightMousePress)}
    {
        assert(mLeftPressedCallback);
        assert(mRightPressedCallback);
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
            Logging::LogDebug("ClickButtonBase") << "Got LMC: " << click << std::endl;
            std::invoke(mLeftPressedCallback);
            return true;
        }

        return false;
    }

    bool RightMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            Logging::LogDebug("ClickButtonBase") << "Got RMC: " << click << std::endl;
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
        const Font& fr,
        const std::string& label,
        std::function<void()>&& onLeftMousePress)
    :
        ClickButtonBase{
            pos,
            dims,
            std::move(onLeftMousePress),
            [](){}
        },
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
        mText{
            glm::vec2{3, 2},
            dims},
        mButton{}
    {
        const auto& [endPos, text] = mText.AddText(fr, label);
        const auto& textPos = mText.GetPositionInfo().mPosition;
        mText.SetPosition(
            glm::vec2{
                textPos.x + (dims.x - endPos.x) / 2,
                textPos.y});
        mButton.emplace_back(&mNormal);
        mButton.emplace_back(&mText);
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
        assert(mButton.size() >= 1);
        if (Within(click))
        {
            mButton[0] = &mPressed;
        }

        return false;
    }

    bool LeftMouseReleased(glm::vec2 click)
    {
        assert(mButton.size() >= 1);
        mButton[0] = &mNormal;
        return false;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!Within(pos))
        {
            assert(mButton.size() >= 1);
            mButton[0] = &mNormal;
        }

        return false;
    }

private:
    Button mNormal;
    Button mPressed;
    TextBox mText;
    
    std::vector<Graphics::IGuiElement*> mButton;
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
        assert(mButton.size() >= 1);
        if (Within(click))
            mButton[0] = &mPressed;

        return false;
    }

    bool LeftMouseReleased(glm::vec2 click)
    {
        assert(mButton.size() >= 1);
        mButton[0] = &mNormal;
        return false;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!Within(pos))
        {
            assert(mButton.size() >= 1);
            mButton[0] = &mNormal;
        }

        return false;
    }


private:
    Widget mNormal;
    Widget mPressed;
    
    std::vector<Graphics::IGuiElement*> mButton;
};

}
