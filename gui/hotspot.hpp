#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/colors.hpp"
#include "gui/clickable.hpp"
#include "gui/cursor.hpp"
#include "gui/highlightable.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

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
        mHighlightCursor{highlightCursor},
        mText{
            glm::vec2{0},
            glm::vec2{50, 16}
        }
    {
        //std::stringstream ss{};
        //ss << id;
        //mText.AddText(font, ss.str());

        //AddChildBack(&mText);
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
    TextBox mText;
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
