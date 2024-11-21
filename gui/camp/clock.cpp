#include "gui/camp/clock.hpp"

#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

namespace Gui::Camp::detail {

CampDest::CampDest(
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

bool CampDest::GetCurrent() const
{
    return mCurrent;
}

void CampDest::SetCurrent(bool current)
{
    mCurrent = current;
    SetTexture(std::get<Graphics::TextureIndex>(
        mCurrent
            ? mIcons.GetEncampIcon(sCurrent)
            : mIcons.GetEncampIcon(sUnlit)));
}

void CampDest::SetHighlighted()
{
    SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetEncampIcon(sHighlighted)));
}

void CampDest::Entered()
{
    SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetEncampIcon(sHighlighted)));
    mCallback(true);
}

void CampDest::Exited()
{
    SetCurrent(mCurrent);
    mCallback(false);
}

}
