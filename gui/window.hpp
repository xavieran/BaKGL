#pragma once

#include "gui/core/widget.hpp"
#include "gui/cursor.hpp"

namespace Graphics {
class SpriteManager;
}

namespace Gui {

class Window : public Widget
{
public:
    Window(
        Graphics::SpriteManager& spriteManager,
        float width,
        float height);
    
    bool OnMouseEvent(const MouseEvent& event) override;
    void ShowCursor();
    void HideCursor();
    Cursor& GetCursor();

private:
    Cursor mCursor;
};

}
