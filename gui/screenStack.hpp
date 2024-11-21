#pragma once

#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Logging {
class Logger;
}

namespace Gui {

// Renders all screens in order from bottom to back,
// only passes input to the back screen
class ScreenStack : public Widget
{
public:

    ScreenStack();
    
    bool OnMouseEvent(const MouseEvent& event) override;
    void PushScreen(Widget* widget);
    void PopScreen();
    Widget* Top() const;
    bool HasChildren();
private:

    const Logging::Logger& mLogger;
};

}
