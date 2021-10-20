#pragma once

#include "gui/widget.hpp"

namespace Gui {

template <typename Base, typename DraggedT>
class DragEndpoint : public Base
{
public:
    using Functor = std::function<void(DraggedT&)>;

    template <typename ...Args>
    DragEndpoint(
        Functor&& callback,
        Args&&... args)
    :
        Base{std::forward<Args>(args)...},
        mCallback{std::move(callback)}
    {}

    bool OnDragEvent(const DragEvent& event) override
    {
        evaluate_if<DragEnded>(event, [&](const auto& e){
            if (Base::Within(e.mValue))
            {
                ASSERT(e.mWidget);
                ASSERT(dynamic_cast<DraggedT*>(e.mWidget) != nullptr);
                std::invoke(
                    mCallback,
                    static_cast<DraggedT&>(*e.mWidget));
            }
        });
        return false;
    }

private:
    Functor mCallback;
};

}
