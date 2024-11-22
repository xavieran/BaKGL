#include "gui/hotspot.hpp"

#include "gui/fontManager.hpp"

#include "gui/core/widget.hpp"

#include "gui/cursor.hpp"

namespace Gui::detail {

HotspotBase::HotspotBase(
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

void HotspotBase::Entered()
{
    mCursor.PushCursor(mHighlightCursor);
}

void HotspotBase::Exited()
{
    mCursor.PopCursor();
}

}
