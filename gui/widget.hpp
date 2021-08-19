#pragma once

#include "com/logger.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/guiTypes.hpp"
#include "graphics/glm.hpp"

namespace Gui {

class Widget : public Graphics::IGuiElement
{
public:
    Widget(
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
            childrenRelative}
    {}

    virtual void LeftMousePress(glm::vec2 click)
    {
        Logging::LogSpam("GUI") << "Got LMC: " << click << std::endl;
        for (auto& c : mChildren)
            c->LeftMousePress(TransformEvent(click));
    }

    virtual void LeftMouseRelease(glm::vec2 click)
    {
        Logging::LogSpam("GUI") << "Got LMR: " << click << std::endl;
        for (auto& c : mChildren)
            c->LeftMouseRelease(TransformEvent(click));
    }

    virtual void RightMousePress(glm::vec2 click)
    {
        for (auto& c : mChildren)
            c->RightMousePress(TransformEvent(click));
    }

    virtual void RightMouseRelease(glm::vec2 click)
    {
        for (auto& c : mChildren)
            c->RightMouseRelease(TransformEvent(click));
    }

    virtual void MouseMoved(glm::vec2 pos)
    {
        for (auto& c : mChildren)
            c->MouseMoved(TransformEvent(pos));
    }

    const Graphics::DrawInfo& GetDrawInfo() const override
    {
        return mDrawInfo;
    }

    const Graphics::PositionInfo& GetPositionInfo() const override
    {
        return mPositionInfo;
    }

    virtual void AddChildFront(Widget* widget)
    {
        assert(std::find(mChildren.begin(), mChildren.end(), widget) 
            == mChildren.end());
        mChildren.insert(mChildren.begin(), widget);
        Graphics::IGuiElement::AddChildFront(
            static_cast<Graphics::IGuiElement*>(widget));
    }

    void AddChildBack(Widget* widget)
    {
        assert(std::find(mChildren.begin(), mChildren.end(), widget) 
            == mChildren.end());
        mChildren.emplace_back(widget);
        Graphics::IGuiElement::AddChildBack(
            static_cast<Graphics::IGuiElement*>(widget));
    }

    void RemoveChild(Widget* elem)
    {
        Graphics::IGuiElement::RemoveChild(elem);
        const auto it = std::find(mChildren.begin(), mChildren.end(), elem);
        assert(it != mChildren.end());
        mChildren.erase(it);
    }

    void ClearChildren()
    {
        Graphics::IGuiElement::ClearChildren();
        mChildren.clear();
    }

    void SetCenter(glm::vec2 pos)
    {
        mPositionInfo.mPosition = pos
            - (mPositionInfo.mDimensions / 2.0f);
    }

    void SetPosition(glm::vec2 pos)
    {
        mPositionInfo.mPosition = pos;
    }

protected:
    bool Within(glm::vec2 click)
    {
        return Graphics::PointWithinRectangle(
            glm::vec2{click},
            glm::vec2{GetPositionInfo().mPosition},
            glm::vec2{GetPositionInfo().mDimensions});
    }

    glm::vec2 TransformEvent(glm::vec2 eventPos)
    {
        if (mPositionInfo.mChildrenRelative)
            return eventPos - GetPositionInfo().mPosition;
        else 
            return eventPos;
    }

    Graphics::DrawInfo mDrawInfo;
    Graphics::PositionInfo mPositionInfo;
    std::vector<Widget*> mChildren;
};

}
