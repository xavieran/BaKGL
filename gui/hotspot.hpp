#pragma once

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class Cursor;
class Font;

namespace detail {

class HotspotBase : public Widget
{
public:
    HotspotBase(
        Cursor& cursor,
        const Font& font,
        glm::vec2 pos,
        glm::vec2 dims,
        unsigned id,
        unsigned highlightCursor);

    void Entered();
    void Exited();
private:
    Cursor& mCursor;
    unsigned mHighlightCursor;
};

}

using Hotspot = Clickable<
    Clickable<
        Highlightable<
            detail::HotspotBase, false>,
        RightMousePress,
        std::function<void()>>,
    LeftMousePress,
    std::function<void()>>;

}
