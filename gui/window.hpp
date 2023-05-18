#pragma once

#include "gui/core/widget.hpp"
#include "gui/cursor.hpp"
#include "gui/colors.hpp"

#include "com/visit.hpp"

namespace Gui {

class Window : public Widget
{
public:
    Window(
        Graphics::SpriteManager& spriteManager,
        float width,
        float height)
    :
        Widget{
            RectTag{},
            glm::vec2{0},
            glm::vec2{width, height},
            glm::vec4{0},
            true},
        mCursor{spriteManager}
    {
        AddChildBack(&mCursor);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        Logging::LogSpam("Window") << __FUNCTION__ << " " << event << "\n";
        return std::visit(overloaded{
            [this](const MouseMove& p){
                mCursor.SetPosition(p.mValue);
                return Widget::OnMouseEvent(p);
            },
            [this](const auto& p){
                return Widget::OnMouseEvent(p);
            }},
            event);
    }

    void ShowCursor()
    {
        AddChildBack(&mCursor);
    }

    void HideCursor()
    {
        RemoveChild(&mCursor);
    }

    Cursor& GetCursor()
    {
        return mCursor;
    }

private:
    Cursor mCursor;
};

}
