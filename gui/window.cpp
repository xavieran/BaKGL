#include "gui/window.hpp"

#include "gui/core/widget.hpp"
#include "gui/cursor.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

namespace Gui {

Window::Window(
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

bool Window::OnMouseEvent(const MouseEvent& event)
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

void Window::ShowCursor()
{
    AddChildBack(&mCursor);
}

void Window::HideCursor()
{
    RemoveChild(&mCursor);
}

Cursor& Window::GetCursor()
{
    return mCursor;
}

}

