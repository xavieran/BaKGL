#pragma once

#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

namespace Gui::Camp {

namespace detail {

class CampDest : public Widget
{
    static constexpr auto sBlank = 0;
    static constexpr auto sUnlit = 1;
    static constexpr auto sHighlighted = 2;
    static constexpr auto sCurrent = 3;
    static constexpr auto sBlank2 = 4;

public:
    CampDest(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(bool)>&& selected)
    :
        Widget{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetEncampIcon(sUnlit)),
            std::get<Graphics::TextureIndex>(icons.GetEncampIcon(sUnlit)),
            pos,
            {8, 3},
            false
        },
        mIcons{icons},
        mCurrent{},
        mCallback{std::move(selected)}
    {
    }

    bool GetCurrent() const
    {
        return mCurrent;
    }

    void SetCurrent(bool current)
    {
        mCurrent = current;
        SetTexture(std::get<Graphics::TextureIndex>(
            mCurrent
                ? mIcons.GetEncampIcon(sCurrent)
                : mIcons.GetEncampIcon(sUnlit)));
    }

    void SetHighlighted()
    {
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetEncampIcon(sHighlighted)));
    }

public:
    void Entered()
    {
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetEncampIcon(sHighlighted)));
        mCallback(true);
    }

    void Exited()
    {
        SetCurrent(mCurrent);
        mCallback(false);
    }

    const Icons& mIcons;
    bool mCurrent;
    std::function<void(bool)> mCallback;
};

}
}
