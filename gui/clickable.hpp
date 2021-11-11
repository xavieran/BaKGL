#pragma once

#include "gui/widget.hpp"

#include "com/assert.hpp"
#include "com/visit.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

template <typename Base, typename EventT, typename Functor>
class Clickable : public Base
{
public:
    template <typename ...Args>
    Clickable(Functor&& callback, Args&&... args)
    :
        Base{std::forward<Args>(args)...},
        mCallback{std::move(callback)}
    {
        ASSERT(mCallback);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        const auto result = std::visit(overloaded{
            [this](const EventT& p){ return DoCallback(p); },
            [](const auto& p){ return false; }
            },
            event);

        if (result)
            return result;

        return Base::OnMouseEvent(event);
    }

    bool DoCallback(const EventT& event)
    {
        if (Base::Within(event.mValue))
        {
            mCallback();
            return true;
        }

        return false;
    }

private:
    Functor mCallback;
};

}
