#include "gui/widget.hpp"

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

void Widget::SetActive()
{
    mActive = true;
}

void Widget::SetInactive()
{
    mActive = false;
}

void Widget::LeftMousePress(glm::vec2 click)
{
    if (mActive)
        for (auto& c : mChildren)
            c->LeftMousePress(TransformEvent(click));
}

void Widget::LeftMouseRelease(glm::vec2 click)
{
    if (mActive)
        for (auto& c : mChildren)
            c->LeftMouseRelease(TransformEvent(click));
}

void Widget::RightMousePress(glm::vec2 click)
{
    if (mActive)
        for (auto& c : mChildren)
            c->RightMousePress(TransformEvent(click));
}

void Widget::RightMouseRelease(glm::vec2 click)
{
    if (mActive)
        for (auto& c : mChildren)
            c->RightMouseRelease(TransformEvent(click));
}

void Widget::MouseMoved(glm::vec2 pos)
{
    if (mActive)
        for (auto& c : mChildren)
            c->MouseMoved(TransformEvent(pos));
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
void Widget::SetDimensions(glm::vec2 dims)
{
    mPositionInfo.mDimensions = dims;
}

bool Widget::Within(glm::vec2 click)
{
    return Graphics::PointWithinRectangle(
        glm::vec2{click},
        glm::vec2{GetPositionInfo().mPosition},
        glm::vec2{GetPositionInfo().mDimensions});
}

glm::vec2 Widget::TransformEvent(glm::vec2 eventPos)
{
    if (mPositionInfo.mChildrenRelative)
        return eventPos - GetPositionInfo().mPosition;
    else 
        return eventPos;
}

}
