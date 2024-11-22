#pragma once

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

#include <functional>

namespace Gui {

class Icons;

namespace detail {

class TeleportDest : public Widget
{
    static constexpr auto sNormalIcon = 15;
    static constexpr auto sHighlightedIcon = 13;

public:
    TeleportDest(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(bool)>&& selected);

    void SetSelected();
    void SetUnselected();
    bool OnMouseEvent(const MouseEvent& event) override;
    bool IsCanReach() const;
    void SetCanReach(bool canReach);

//protected: would be great if concept could enforce
// existence of protected members...
public:
    void Entered();
    void Exited();

    const Icons& mIcons;
    bool mSelected;
    bool mCanReach;
    std::function<void(bool)> mCallback;
};

}

using TeleportDest = Highlightable<
    Clickable<
        detail::TeleportDest,
        LeftMousePress,
        std::function<void()>>,
    true>;

}
