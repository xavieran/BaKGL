#include "gui/widget.hpp"

#include "com/assert.hpp"

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
    mParent{nullptr},
    mChildren{},
    mActive{true}
{}

Widget::Widget(
    ImageTag,
    Graphics::SpriteSheetIndex spriteSheet,
    Graphics::TextureIndex texture,
    glm::vec2 pos,
    glm::vec2 dims,
    bool childrenRelative)
:
    Widget{
        Graphics::DrawMode::Sprite,
        spriteSheet,
        texture,
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        pos,
        dims,
        childrenRelative
    }
{
}

Widget::Widget(
    ClipRegionTag,
    glm::vec2 pos,
    glm::vec2 dims,
    bool childrenRelative)
:
    Widget{
        Graphics::DrawMode::ClipRegion,
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        glm::vec4{1},
        pos,
        dims,
        childrenRelative
    }
{
}

Widget::Widget(
    RectTag,
    glm::vec2 pos,
    glm::vec2 dims,
    glm::vec4 color,
    bool childrenRelative)
:
    Widget{
        Graphics::DrawMode::Rect,
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        color,
        pos,
        dims,
        childrenRelative
    }
{
}

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

bool Widget::OnDragEvent(const DragEvent& event)
{
    if (mActive)
    {
        for (auto& c : mChildren)
        {
            const bool handled = c->OnDragEvent(
                TransformEvent(event));
            if (handled)
                return true;
        }
    }

    return false;
}

void Widget::PropagateUp(const DragEvent& event)
{
    if (mActive && mParent != nullptr)
    {
        // When the event arrives at the parent it should be in the same
        // space as the parent
        mParent->PropagateUp(
            mParent->InverseTransformEvent(event));
    }
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
    ASSERT(std::find(mChildren.begin(), mChildren.end(), widget) 
        == mChildren.end());
    mChildren.insert(mChildren.begin(), widget);
    widget->SetParent(this);

    Graphics::IGuiElement::AddChildFront(
        static_cast<Graphics::IGuiElement*>(widget));
}

void Widget::AddChildBack(Widget* widget)
{
    ASSERT(std::find(mChildren.begin(), mChildren.end(), widget) 
        == mChildren.end());
    mChildren.emplace_back(widget);
    widget->SetParent(this);

    Graphics::IGuiElement::AddChildBack(
        static_cast<Graphics::IGuiElement*>(widget));
}

void Widget::RemoveChild(Widget* elem)
{
    elem->SetParent(nullptr);

    Graphics::IGuiElement::RemoveChild(elem);
    const auto it = std::find(mChildren.begin(), mChildren.end(), elem);
    ASSERT(it != mChildren.end());
    mChildren.erase(it);
}

void Widget::PopChild()
{
    if (mChildren.size() > 0)
    {
        auto widget = mChildren.back();
        RemoveChild(widget);
    }
}

void Widget::ClearChildren()
{
    Graphics::IGuiElement::ClearChildren();

    for (auto* child : mChildren)
        child->SetParent(nullptr);

    mChildren.clear();
}

void Widget::SetParent(Widget* widget)
{
    mParent = widget;
}

void Widget::SetCenter(glm::vec2 pos)
{
    mPositionInfo.mPosition = pos
        - (mPositionInfo.mDimensions / 2.0f);
}

glm::vec2 Widget::GetCenter() const
{
    return mPositionInfo.mPosition 
        + (mPositionInfo.mDimensions / 2.0f);
}

glm::vec2 Widget::GetTopLeft() const
{
    return mPositionInfo.mPosition;
}

void Widget::SetPosition(glm::vec2 pos)
{
    mPositionInfo.mPosition = pos;
}

void Widget::AdjustPosition(glm::vec2 adj)
{
    mPositionInfo.mPosition += adj;
}

void Widget::SetSpriteSheet(Graphics::SpriteSheetIndex spriteSheet)
{
    mDrawInfo.mSpriteSheet = spriteSheet;
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

glm::vec2 Widget::TransformPosition(const glm::vec2& pos)
{
    if (mPositionInfo.mChildrenRelative)
        return pos - GetPositionInfo().mPosition;
    else 
        return pos;
}

glm::vec2 Widget::InverseTransformPosition(const glm::vec2& pos)
{
    if (mPositionInfo.mChildrenRelative)
        return pos + GetPositionInfo().mPosition;
    else 
        return pos;
}

// When propagating event from parent to child
MouseEvent Widget::TransformEvent(const MouseEvent& event)
{
    return std::visit(
        [&]<typename T>(const T& e) -> MouseEvent
        {
            const auto newPos = TransformPosition(e.mValue);
            return MouseEvent{T{newPos}};
        },
        event);
}

DragEvent Widget::TransformEvent(const DragEvent& event)
{
    return std::visit(
        [&]<typename T>(const T& e) -> DragEvent
        {
            const auto newPos = TransformPosition(e.mValue);
            return DragEvent{T{e.mWidget, newPos}};
        },
        event);
}

DragEvent Widget::InverseTransformEvent(const DragEvent& event)
{
    return std::visit(
        [&]<typename T>(const T& e) -> DragEvent
        {
            const auto newPos = InverseTransformPosition(e.mValue);
            return DragEvent{T{e.mWidget, newPos}};
        },
        event);
}

}
