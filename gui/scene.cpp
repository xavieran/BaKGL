#include "gui/scene.hpp"

namespace Gui {

EnableClipRegion ConvertScriptAction(
    const BAK::ClipRegion& clip)
{
    const auto width  = clip.mBottomRight.x - clip.mTopLeft.x;
    const auto height = clip.mBottomRight.y - clip.mTopLeft.y;

    return EnableClipRegion{
        glm::vec2{
            clip.mTopLeft.x,
            clip.mTopLeft.y},
        glm::vec2{width, height}
    };
}

DisableClipRegion ConvertScriptAction(
    const BAK::DisableClipRegion&)
{
    return DisableClipRegion{};
}

}
