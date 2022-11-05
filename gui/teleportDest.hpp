#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/widget.hpp"
#include "gui/highlightable.hpp"
#include "gui/clickable.hpp"
#include "gui/icons.hpp"

#include <functional>

namespace Gui {

namespace detail {

class TeleportDest : public Widget
{
    static constexpr auto sNormalIcon = 15;
    static constexpr auto sHighlightedIcon = 13;

public:
    TeleportDest(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(bool)>&& selected)
    :
        Widget{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetTeleportIcon(sNormalIcon)),
            std::get<Graphics::TextureIndex>(icons.GetTeleportIcon(sNormalIcon)),
            pos,
            std::get<glm::vec2>(icons.GetTeleportIcon(sNormalIcon)),
            false
        },
        mIcons{icons},
        mSelected{},
        mCallback{selected}
    {
    }

    void SetSelected()
    {
        mSelected = true;
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sHighlightedIcon)));
    }

    void SetUnselected()
    {
        mSelected = false;
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sNormalIcon)));
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return false;
    }

protected:
    void Entered()
    {
        if (mSelected) return;
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sHighlightedIcon)));
        mCallback(true);
    }

    void Exited()
    {
        if (mSelected) return;
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sNormalIcon)));
        mCallback(false);
    }

    const Icons& mIcons;
    bool mSelected;
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
