#pragma once

#include "gui/colors.hpp"
#include "gui/widget.hpp"

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

    void LeftMousePress(glm::vec2 click) override
    {
        if (mChildren.size() > 0)
            mChildren.back()->LeftMousePress(click);
    }

    void LeftMouseRelease(glm::vec2 click) override
    {
        if (mChildren.size() > 0)
            mChildren.back()->LeftMouseRelease(click);
    }

    void RightMousePress(glm::vec2 click) override
    {
        if (mChildren.size() > 0)
            mChildren.back()->RightMousePress(click);
    }

    void RightMouseRelease(glm::vec2 click) override
    {
        if (mChildren.size() > 0)
            mChildren.back()->RightMouseRelease(click);
    }

    void MouseMoved(glm::vec2 pos) override
    {
        if (mChildren.size() > 0)
        {
            mChildren.back()->MouseMoved(pos);
        }
    }

    void PushScreen(Widget* widget)
    {
        mLogger.Debug() << "Pushed widget " << std::hex << widget << std::dec << "\n";
        AddChildBack(widget);
    }

    void PopScreen()
    {
        assert(mChildren.size() > 0);
        mLogger.Debug() << "Popped widget: "  << std::hex << mChildren.back() << std::dec << "\n";
        PopChild();
    }

private:

    const Logging::Logger& mLogger;
};

}
