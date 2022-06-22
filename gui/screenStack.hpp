#pragma once

#include "gui/colors.hpp"
#include "gui/widget.hpp"

#include "com/ostream.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {


// Renders all screens in order from bottom to back,
// only passes input to the back screen
class ScreenStack : public Widget
{
public:

    ScreenStack()
    :
        Widget{
            Graphics::DrawMode::Rect,
            Graphics::SpriteSheetIndex{0},
            Graphics::TextureIndex{0},
            Graphics::ColorMode::SolidColor,
            Color::debug,
            glm::vec2{0},
            glm::vec2{1},
            false
        },
        mLogger{Logging::LogState::GetLogger("Gui::ScreenStack")}
    {
        mLogger.Debug() << "Constructed @" << std::hex << this << std::dec << "\n";
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        if (mChildren.size() > 0)
        {
            if (mChildren.back()->OnMouseEvent(event))
                return true;
        }
        return false;
    }

    void PushScreen(Widget* widget)
    {
        mLogger.Debug() << "Widgets: " << GetChildren() << "\n";
        mLogger.Debug() << "Pushed widget " << std::hex << widget << std::dec << "\n";
        AddChildBack(widget);
    }

    void PopScreen()
    {
        ASSERT(mChildren.size() > 0);
        mLogger.Debug() << "Popped widget: "  << std::hex << mChildren.back() << std::dec << "\n";
        PopChild();
    }

    Widget* Top()
    {
        assert(mChildren.size() > 0);
        return mChildren.back();
    }

    bool HasChildren()
    {
        return mChildren.size() > 0;
    }

private:

    const Logging::Logger& mLogger;
};

}
