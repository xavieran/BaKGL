#pragma once

#include "com/logger.hpp"

#include "gui/widget.hpp"
#include "gui/colors.hpp"
#include "gui/cursor.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

namespace Gui {

class Hotspot : public Widget
{
public:
    Hotspot(
        Cursor& cursor,
        glm::vec2 pos,
        glm::vec2 dims,
        unsigned highlightCursor,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress)
    :
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            //glm::vec4{0},
            Color::debug,
            // Filthy hack - make these a little smaller
            // because some of them overlap which breaks the cursor
            pos + glm::vec2{2},
            dims - glm::vec2{3},
            true
        },
        mCursor{cursor},
        mHighlightCursor{highlightCursor},
        mLeftPressed{std::move(onLeftMousePress)},
        mRightPressed{std::move(onRightMousePress)},
        mWithinWidget{}
    {
    }

    void LeftMousePress(glm::vec2 click) override
    {
        Logging::LogDebug("ClickButton") << "Got LMC: " << click << std::endl;
        if (Within(click))
        {
            if (mLeftPressed)
                std::invoke(mLeftPressed);
        }
    }

    void RightMousePress(glm::vec2 click) override
    {
        Logging::LogDebug("ClickButton") << "Got RMC: " << click << std::endl;
        if (Within(click))
        {
            if (mRightPressed)
                std::invoke(mRightPressed);
        }
    }

    void MouseMoved(glm::vec2 pos) override
    {
        if (!mWithinWidget)
        {
            if (Within(pos))
            {
                mWithinWidget = true;
                mCursor.PushCursor(mHighlightCursor);
            }
            else
            {
                mWithinWidget = false;
            }
        }
        else
        {
            // Mouse entered widget
            if (Within(pos) && !(*mWithinWidget))
            {
                Logging::LogSpam("Gui::Hotspot") << " Entered( "  << mHighlightCursor << " )\n";
                mWithinWidget = true;
                mCursor.PushCursor(mHighlightCursor);
            }
            // Mouse exited widget
            else if (!Within(pos) && *mWithinWidget)
            {
                Logging::LogSpam("Gui::Hotspot") << " Exited ( "  << mHighlightCursor << " )\n";
                mWithinWidget = false;
                mCursor.PopCursor();
            }
        }
    }

private:
    Cursor& mCursor;
    unsigned mHighlightCursor;

    std::function<void()> mLeftPressed;
    std::function<void()> mRightPressed;

    std::optional<bool> mWithinWidget;
};

}
