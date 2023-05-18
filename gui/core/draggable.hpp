#pragma once

#include "gui/core/widget.hpp"

#include "com/visit.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

namespace Gui {

template <typename Base>
class Draggable : public Base
{
public:
    template <typename ...Args>
    Draggable(Args&&... args)
    :
        Base{std::forward<Args>(args)...},
        mOriginalPosition{Base::GetPositionInfo().mPosition},
        mDragStart{},
        mDragging{false}
    {
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        const auto result = std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const LeftMouseRelease& p){ return LeftMouseReleased(p.mValue); },
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [this](const RightMousePress& p){ return HandleRightMouse(); },
            [this](const RightMouseRelease& p){ return HandleRightMouse(); },
            [](const auto& p){ return false; }
            },
            event);

        if (result)
            return result;

        return Base::OnMouseEvent(event);
    }

    // If this widget is being dragged it 
    // shouldn't accept right mouse events
    bool HandleRightMouse()
    {
        return mDragging;
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        if (Base::Within(click))
        {

            Logging::LogDebug("Gui::Draggable") << "DragStart: " << this << "\n";
            mDragStart = click;
        }

        return false;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!mDragging 
            && mDragStart 
            && glm::distance(*mDragStart, pos) > 4)
        {
            mDragging = true;
            Widget::PropagateUp(DragEvent{DragStarted{this, pos}});
        }

        if (mDragging)
        {
            Base::SetCenter(pos);
        }
        
        return false;
    }

    bool LeftMouseReleased(glm::vec2 click)
    {
        mDragStart.reset();

        if (mDragging)
        {
            Logging::LogDebug("Gui::Draggable") << "DragEnded: " << this << "\n";
            mDragging = false;
            Base::SetPosition(mOriginalPosition);
            Base::PropagateUp(DragEvent{DragEnded{this, click}});
        }

        return false;
    }

private:
    glm::vec2 mOriginalPosition;
    std::optional<glm::vec2> mDragStart;
    bool mDragging;
};

}
