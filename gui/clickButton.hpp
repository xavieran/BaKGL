#pragma once

#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

namespace Gui {

class ClickButton : public Widget
{
public:
    ClickButton(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& fr,
        const std::string& label,
        std::function<void()>&& onLeftMousePress)
    :
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Color::black,
            pos,
            dims,
            true
        },
        mNormal{
            glm::vec2{0,0},
            dims,
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow},
        mPressed{
            glm::vec2{0,0},
            dims,
            Color::buttonPressed,
            Color::buttonShadow,
            Color::buttonHighlight},
        mText{
            glm::vec2{3,3},
            dims},
        mPressedCallback{std::move(onLeftMousePress)},
        mButton{}
    {
        mText.AddText(fr, label);
        mButton.emplace_back(&mNormal);
        mButton.emplace_back(&mText);
    }

    const std::vector<Graphics::IGuiElement*>& GetChildren() const override
    {
        return mButton;
    }

    void LeftMousePress(glm::vec2 click) override
    {
        Logging::LogDebug("ClickButton") << "Got LMC: " << click << std::endl;
        assert(mButton.size() >= 1);
        if (Within(click))
            mButton[0] = &mPressed;
    }

    void MouseMoved(glm::vec2 pos) override
    {
        if (!Within(pos))
        {
            assert(mButton.size() >= 1);
            mButton[0] = &mNormal;
        }
    }

    void LeftMouseRelease(glm::vec2 click) override
    {
        Logging::LogDebug("ClickButton") << "Got LMR: " << click << std::endl;
        assert(mButton.size() >= 1);
        mButton[0] = &mNormal;

        if (Within(click))
        {
            if (mPressedCallback)
                std::invoke(mPressedCallback);
        }
    }

private:
    Button mNormal;
    Button mPressed;
    TextBox mText;
    
    std::function<void()> mPressedCallback;
    std::vector<Graphics::IGuiElement*> mButton;
};

class ClickButtonImage : public Widget
{
public:
    ClickButtonImage(
        glm::vec2 pos,
        glm::vec2 dims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex normal,
        Graphics::TextureIndex pressed,
        std::function<void()>&& onLeftMousePress)
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
        mPressedCallback{std::move(onLeftMousePress)},
        mButton{}
    {
        mButton.emplace_back(&mNormal);
    }

    const std::vector<Graphics::IGuiElement*>& GetChildren() const override
    {
        return mButton;
    }

    void LeftMousePress(glm::vec2 click) override
    {
        Logging::LogDebug("ClickButton") << "Got LMC: " << click << std::endl;
        assert(mButton.size() >= 1);
        if (Within(click))
            mButton[0] = &mPressed;
    }

    void MouseMoved(glm::vec2 pos) override
    {
        if (!Within(pos))
        {
            assert(mButton.size() >= 1);
            mButton[0] = &mNormal;
        }
    }

    void LeftMouseRelease(glm::vec2 click) override
    {
        Logging::LogDebug("ClickButton") << "Got LMR: " << click << std::endl;
        assert(mButton.size() >= 1);
        mButton[0] = &mNormal;

        if (Within(click))
        {
            if (mPressedCallback)
                std::invoke(mPressedCallback);
        }
    }

private:
    Widget mNormal;
    Widget mPressed;
    
    std::function<void()> mPressedCallback;
    std::vector<Graphics::IGuiElement*> mButton;
};

}
