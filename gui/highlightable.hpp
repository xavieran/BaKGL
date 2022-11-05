#pragma once

#include "gui/widget.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <optional>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

template <typename Base, bool HandleBaseFirst>
class Highlightable : public Base
{
public:
    template <typename ...Args>
    Highlightable(Args&&... args)
    :
        Base{std::forward<Args>(args)...},
        mWithinWidget{}
    {
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        std::visit(overloaded{
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [](const auto& p){ return false; }
            }, event);

        return Base::OnMouseEvent(event);
    }

private:
    bool MouseMoved(glm::vec2 pos)
    {
        if (!mWithinWidget)
        {
            if (Base::Within(pos))
            {
                Base::Entered();
                mWithinWidget = true;
            }
            else
            {
                Base::Exited();
                mWithinWidget = false;
            }
        }
        else
        {
            // Mouse entered widget
            if (Base::Within(pos) && !(*mWithinWidget))
            {
                Base::Entered();
                mWithinWidget = true;
            }
            // Mouse exited widget
            else if (!Base::Within(pos) && *mWithinWidget)
            {
                Base::Exited();
                mWithinWidget = false;
            }
        }

        return false;
    }

private:
    std::optional<bool> mWithinWidget;
};

}
