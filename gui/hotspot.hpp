#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

#include "gui/colors.hpp"
#include "gui/cursor.hpp"
#include "gui/textBox.hpp"

namespace Gui {

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
        unsigned highlightCursor)
    :
        Widget{
            RectTag{},
            // Filthy hack - make these a little smaller
            // because some of them overlap which breaks the cursor
            pos + glm::vec2{2},
            dims - glm::vec2{3},
            //Color::debug,
            glm::vec4{0},
            true
        },
        mCursor{cursor},
        mHighlightCursor{highlightCursor}
    {
    }

    void Entered()
    {
        mCursor.PushCursor(mHighlightCursor);
    }

    void Exited()
    {
        mCursor.PopCursor();
    }

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
