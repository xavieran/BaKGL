#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/widget.hpp"
#include "gui/colors.hpp"
#include "gui/cursor.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

namespace Gui {

class TownLabel : public Widget
{
public:
    TownLabel(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const std::string& label)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{0},
            true
        },
        mTown{label},
        mLabel{{}, glm::vec2{120, 32}}
    {
        const auto& [tDims, _] = mLabel.AddText(font, label);
        mLabel.SetDimensions(tDims);
        mLabel.SetCenter(glm::vec2{0, -3});
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    bool MouseMoved(glm::vec2 pos)
    {
        if (!mWithinWidget)
        {
            if (Within(pos))
            {
                mWithinWidget = true;
                ClearChildren();
                AddChildBack(&mLabel);
            }
            else
            {
                ClearChildren();
                mWithinWidget = false;
            }
        }
        else
        {
            // Mouse entered widget
            if (Within(pos) && !(*mWithinWidget))
            {
                Logging::LogSpam("Gui::TownLabel") << " Entered ( "  << mTown << ")\n";
                mWithinWidget = true;
                ClearChildren();
                AddChildBack(&mLabel);
            }
            // Mouse exited widget
            else if (!Within(pos) && *mWithinWidget)
            {
                Logging::LogSpam("Gui::TownLabel") << " Exited ( "  << mTown << ")\n";
                ClearChildren();
                mWithinWidget = false;
            }
        }

        return false;
    }

private:
    std::optional<bool> mWithinWidget;
    std::string mTown;
    TextBox mLabel;
};

}
