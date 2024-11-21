#pragma once

#include "gui/core/widget.hpp"

#include <stack>

namespace Logging {
class Logger;
}

namespace Graphics {
class Sprites;
class SpriteManager;
}

namespace Gui {

class Cursor : public Widget
{
public:
    using Dimensions = glm::vec2;
    using CursorIndex = unsigned;
    using CursorInfo = std::pair<Dimensions, CursorIndex>;

    Cursor(
        Graphics::SpriteManager& spriteManager);
    
    void Clear();
    void PushCursor(unsigned cursor);
    void PopCursor();
    void Hide();
    void Show();
    const CursorInfo& GetCursor();
    const Graphics::Sprites& GetSprites();
private:
    void UpdateCursor();

    std::stack<CursorInfo> mCursors;
    Graphics::Sprites& mSprites;
    const Logging::Logger& mLogger;
};

}
