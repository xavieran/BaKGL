#include "gui/screenStack.hpp"

#include "gui/core/widget.hpp"
#include "gui/colors.hpp"

#include "com/ostream.hpp"
#include "com/logger.hpp"

#include <glm/glm.hpp>

namespace Gui {

ScreenStack::ScreenStack()
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

bool ScreenStack::OnMouseEvent(const MouseEvent& event)
{
    if (mChildren.size() > 0)
    {
        if (mChildren.back()->OnMouseEvent(event))
            return true;
    }
    return false;
}

void ScreenStack::PushScreen(Widget* widget)
{
    mLogger.Debug() << "Widgets: " << GetChildren() << " Pushed widget " << std::hex << widget << std::dec << "\n";
    AddChildBack(widget);
}

void ScreenStack::PopScreen()
{
    ASSERT(mChildren.size() > 0);
    mLogger.Debug() << "Popped widget: "  << std::hex << mChildren.back() << std::dec << "\n";
    PopChild();
}

Widget* ScreenStack::Top() const
{
    assert(mChildren.size() > 0);
    return mChildren.back();
}

bool ScreenStack::HasChildren()
{
    return mChildren.size() > 0;
}

}
