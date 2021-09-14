#pragma once

#include "gui/widget.hpp"
#include "gui/cursor.hpp"
#include "gui/colors.hpp"

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
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            glm::vec2{0},
            glm::vec2{width, height},
            true},
        mCursor{spriteManager}
    {
        AddChildBack(&mCursor);
    }

    void MouseMoved(glm::vec2 pos) override
    {
        mCursor.SetPosition(pos);
        Widget::MouseMoved(pos);
    }

    Cursor& GetCursor()
    {
        return mCursor;
    }

private:
    Cursor mCursor;
};

}
