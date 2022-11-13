#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

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
        const Font& font,
        glm::vec2 pos,
        glm::vec2 dims,
        unsigned id,
        unsigned highlightCursor,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress)
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
        },
        mLeftPressed{std::move(onLeftMousePress)},
        mRightPressed{std::move(onRightMousePress)},
        mWithinWidget{}
    {
        //std::stringstream ss{};
        //ss << id;
        //mText.AddText(font, ss.str());

        //AddChildBack(&mText);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const RightMousePress& p){ return RightMousePressed(p.mValue); },
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        Logging::LogDebug("ClickButton") << "Got LMC: " << click << std::endl;
        if (Within(click))
        {
            if (mLeftPressed)
            {
                std::invoke(mLeftPressed);
                return true;
            }
        }
        return false;
    }

    bool RightMousePressed(glm::vec2 click)
    {
        Logging::LogDebug("ClickButton") << "Got RMC: " << click << std::endl;
        if (Within(click))
        {
            if (mRightPressed)
            {
                std::invoke(mRightPressed);
                return true;
            }
        }
        return false;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!mWithinWidget)
        {
            if (Within(pos))
            {
                mWithinWidget = true;
                Logging::LogSpam("Gui::Hotspot") << " EnteredFrom( "  << mHighlightCursor << " )\n";
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

        return false;
    }

private:
    Cursor& mCursor;
    unsigned mHighlightCursor;
    TextBox mText;

    std::function<void()> mLeftPressed;
    std::function<void()> mRightPressed;

    std::optional<bool> mWithinWidget;
};

}
