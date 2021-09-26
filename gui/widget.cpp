#include "gui/widget.hpp"

#include <variant>

namespace Gui {

Widget::Widget(
    Graphics::DrawMode drawMode,
    Graphics::SpriteSheetIndex spriteSheet,
    Graphics::TextureIndex texture,
    Graphics::ColorMode colorMode,
    glm::vec4 color,
    glm::vec2 pos,
    glm::vec2 dims,
    bool childrenRelative)
:
    mDrawInfo{
        drawMode,
        spriteSheet,
        texture,
        colorMode,
        color},
    mPositionInfo{
        pos,
        dims,
        childrenRelative},
    mActive{true}
{}

Widget::~Widget()
{
}

void Widget::SetActive()
{
    mActive = true;
}

void Widget::SetInactive()
{
    mActive = false;
}

bool Widget::OnMouseEvent(const MouseEvent& event)
{
    if (mActive)
    {
        for (auto& c : mChildren)
        {
            const bool handled = c->OnMouseEvent(
                TransformEvent(event));
            if (handled)
                return true;
        }
    }
    return false;
}

const Graphics::DrawInfo& Widget::GetDrawInfo() const
{
    return mDrawInfo;
}

const Graphics::PositionInfo& Widget::GetPositionInfo() const
{
    return mPositionInfo;
}

void Widget::AddChildFront(Widget* widget)
{
    assert(std::find(mChildren.begin(), mChildren.end(), widget) 
        == mChildren.end());
    mChildren.insert(mChildren.begin(), widget);
    Graphics::IGuiElement::AddChildFront(
        static_cast<Graphics::IGuiElement*>(widget));
}

void Widget::AddChildBack(Widget* widget)
{
    assert(std::find(mChildren.begin(), mChildren.end(), widget) 
        == mChildren.end());
    mChildren.emplace_back(widget);
    Graphics::IGuiElement::AddChildBack(
        static_cast<Graphics::IGuiElement*>(widget));
}

void Widget::PopChild()
{
    if (mChildren.size() > 0)
    {
        auto widget = mChildren.back();
        RemoveChild(widget);
    }
}

void Widget::RemoveChild(Widget* elem)
{
    Graphics::IGuiElement::RemoveChild(elem);
    const auto it = std::find(mChildren.begin(), mChildren.end(), elem);
    assert(it != mChildren.end());
    mChildren.erase(it);
}

void Widget::ClearChildren()
{
    Graphics::IGuiElement::ClearChildren();
    mChildren.clear();
}

void Widget::SetCenter(glm::vec2 pos)
{
    mPositionInfo.mPosition = pos
        - (mPositionInfo.mDimensions / 2.0f);
}

glm::vec2 Widget::GetCenter()
{
    return mPositionInfo.mPosition 
        + (mPositionInfo.mDimensions / 2.0f);
}

void Widget::SetPosition(glm::vec2 pos)
{
    mPositionInfo.mPosition = pos;
}

void Widget::AdjustPosition(glm::vec2 adj)
{
    mPositionInfo.mPosition += adj;
}

void Widget::SetTexture(Graphics::TextureIndex texture)
{
    mDrawInfo.mTexture = texture;
}

void Widget::SetColorMode(Graphics::ColorMode cm)
{
    mDrawInfo.mColorMode = cm;
}

void Widget::SetColor(glm::vec4 color)
{
    mDrawInfo.mColor = color;
}

void Widget::SetDimensions(glm::vec2 dims)
{
    mPositionInfo.mDimensions = dims;
}

std::size_t Widget::size() const
{
    return mChildren.size();
}

bool Widget::Within(glm::vec2 click)
{
    return Graphics::PointWithinRectangle(
        glm::vec2{click},
        glm::vec2{GetPositionInfo().mPosition},
        glm::vec2{GetPositionInfo().mDimensions});
}

MouseEvent Widget::TransformEvent(const MouseEvent& event)
{
    const auto& pos = std::visit(
        [](const auto& e)
        {
            return e.mValue;
        },
        event);

    const auto newPos = std::invoke([&]{
        if (mPositionInfo.mChildrenRelative)
            return pos - GetPositionInfo().mPosition;
        else 
            return pos;
    });

    return std::visit(
        [&]<typename T>(const T&) -> MouseEvent
        {
            return MouseEvent{T{newPos}};
        },
        event);
}

}
