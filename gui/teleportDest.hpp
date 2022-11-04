#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/widget.hpp"
#include "gui/icons.hpp"

#include <functional>

namespace Gui {

class TeleportDest : public Widget
{
    static constexpr auto sNormalIcon = 15;
    static constexpr auto sHighlightedIcon = 13;

public:
    TeleportDest(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(bool)>&& selected,
        std::function<void()>&& pressed)
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
        mWithinWidget{},
        mSelected{},
        mCallback{selected},
        mPressed{pressed}
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
        return std::visit(overloaded{
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [this](const LeftMousePress& p){ return MousePressed(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    // FIXME: Refactor this with townLabel and hotspot
    bool MouseMoved(glm::vec2 pos)
    {
        if (mSelected)
            return false;

        if (!mWithinWidget)
        {
            if (Within(pos))
            {
                Entered();
                mWithinWidget = true;
            }
            else
            {
                Exited();
                mWithinWidget = false;
            }
        }
        else
        {
            // Mouse entered widget
            if (Within(pos) && !(*mWithinWidget))
            {
                Entered();
                mWithinWidget = true;
            }
            // Mouse exited widget
            else if (!Within(pos) && *mWithinWidget)
            {
                Exited();
                mWithinWidget = false;
            }
        }

        return false;
    }

    // Use Clickable here..
    bool MousePressed(glm::vec2 pos)
    {
        if (Within(pos))
        {
            mPressed();
            return true;
        }
        return false;
    }

protected:
    void Entered()
    {
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sHighlightedIcon)));
        mCallback(true);
    }

    void Exited()
    {
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sNormalIcon)));
        mCallback(false);
    }

    const Icons& mIcons;
    std::optional<bool> mWithinWidget;
    bool mSelected;
    std::function<void(bool)> mCallback;
    std::function<void()> mPressed;
};

}
