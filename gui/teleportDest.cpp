#include "gui/teleportDest.hpp"

#include "gui/icons.hpp"

#include <functional>

namespace Gui::detail  {

TeleportDest::TeleportDest(
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
    mCanReach{},
    mCallback{selected}
{
}

void TeleportDest::SetSelected()
{
    mSelected = true;
    SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sHighlightedIcon)));
}

void TeleportDest::SetUnselected()
{
    mSelected = false;
    SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sNormalIcon)));
}

bool TeleportDest::OnMouseEvent(const MouseEvent& event)
{
    return false;
}

bool TeleportDest::IsCanReach() const
{
    return mCanReach;
}

void TeleportDest::SetCanReach(bool canReach)
{
    mCanReach = canReach;
}

void TeleportDest::Entered()
{
    if (mSelected) return;
    SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sHighlightedIcon)));
    mCallback(true);
}

void TeleportDest::Exited()
{
    if (mSelected) return;
    SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(sNormalIcon)));
    mCallback(false);
}

}
