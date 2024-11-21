#include "gui/clickButton.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include <functional>

namespace Gui {

ClickButtonBase::ClickButtonBase(
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

bool ClickButtonBase::OnMouseEvent(const MouseEvent& event)
{
    return std::visit(overloaded{
        [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
        [this](const RightMousePress& p){ return RightMousePressed(p.mValue); },
        [](const auto& p){ return false; }
        },
        event);
}

bool ClickButtonBase::LeftMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        Logging::LogSpam("ClickButtonBase") << "Got LMC: " << this << " " << click << std::endl;
        std::invoke(mLeftPressedCallback);
        return true;
    }

    return false;
}

bool ClickButtonBase::RightMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        Logging::LogSpam("ClickButtonBase") << "Got RMC: " << this << " " << click << std::endl;
        std::invoke(mRightPressedCallback);
        return true;
    }

    return false;
}


ClickButton::ClickButton(
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

void ClickButton::SetText(std::string_view label)
{
    const auto textPos = glm::vec2{3, 2};
    const auto& dims = GetPositionInfo().mDimensions;
    const auto& [endPos, text] = mText.SetText(mFont, label);
    mText.SetPosition(
        glm::vec2{
            textPos.x + (dims.x - endPos.x) / 2,
            textPos.y});
}

bool ClickButton::OnMouseEvent(const MouseEvent& event)
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

bool ClickButton::LeftMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        return UpdateState(true);
    }

    return false;
}

bool ClickButton::LeftMouseReleased(glm::vec2 click)
{
    UpdateState(false);
    return false;
}

bool ClickButton::MouseMoved(glm::vec2 pos)
{
    if (!Within(pos))
    {
        return UpdateState(false);
    }

    return false;
}

void ClickButton::AddChildren()
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

bool ClickButton::UpdateState(bool newState)
{
    const bool tmp = mButtonPressed;
    mButtonPressed = newState;
    return mButtonPressed != tmp;
}


ClickButtonImage::ClickButtonImage(
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
    mIsPressed{false},
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
        true}
{
    AddChildren();
}

bool ClickButtonImage::OnMouseEvent(const MouseEvent& event)
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

bool ClickButtonImage::LeftMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        mIsPressed = true;
        AddChildren();
    }
    return false;
}

bool ClickButtonImage::LeftMouseReleased(glm::vec2 click)
{
    mIsPressed = false;
    AddChildren();
    return false;
}

bool ClickButtonImage::MouseMoved(glm::vec2 pos)
{
    if (!Within(pos))
    {
        mIsPressed = false;
        AddChildren();
    }

    return false;
}

void ClickButtonImage::CenterImage(glm::vec2 dims)
{
    // Set the image to its normal size and center it
    mNormal.SetDimensions(dims);
    mNormal.SetCenter(GetCenter() - GetTopLeft());
    mPressed.SetDimensions(dims);
    mPressed.SetCenter(GetCenter() - GetTopLeft());
}

void ClickButtonImage::SetTexture(Graphics::SpriteSheetIndex ss, Graphics::TextureIndex ti)
{
    mNormal.SetSpriteSheet(ss);
    mNormal.SetTexture(ti);
    mPressed.SetSpriteSheet(ss);
    mPressed.SetTexture(ti);
}

void ClickButtonImage::SetColor(glm::vec4 color)
{
    mNormal.SetColor(color);
    mPressed.SetColor(color);
}

void ClickButtonImage::SetColorMode(Graphics::ColorMode mode)
{
    mNormal.SetColorMode(mode);
    mPressed.SetColorMode(mode);
}

void ClickButtonImage::AddChildren()
{
    ClearChildren();
    if (mIsPressed)
    {
        AddChildBack(&mPressed);
    }
    else
    {
        AddChildBack(&mNormal);
    }
}

}
