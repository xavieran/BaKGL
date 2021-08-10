#pragma once

#include "bak/scene.hpp"
#include "bak/sceneData.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace Gui {

struct EnableClipRegion
{
    glm::vec<2, int> mTopLeft;
    glm::vec<2, int> mDims;
};

struct DisableClipRegion
{
};

struct SceneSprite
{
    unsigned mImage;
    glm::vec3 mPosition;
    glm::vec3 mScale;
};

struct SceneRect
{
    glm::vec4 mColor;
    glm::vec3 mPosition;
    glm::vec3 mDimensions;
};

using DrawingAction = std::variant<
    EnableClipRegion,
    DisableClipRegion,
    SceneSprite,
    SceneRect>;

DrawingAction ConvertSceneAction(const BAK::SceneAction& action);

EnableClipRegion ConvertSceneAction(const BAK::ClipRegion&);
DisableClipRegion ConvertSceneAction(const BAK::DisableClipRegion&);

template <typename T, typename S>
SceneSprite ConvertSceneAction(
    const BAK::DrawSprite& action,
    const T& textures,
    S& offsets) // make this const
{
    const auto sprite = action.mSpriteIndex 
        + offsets[action.mImageSlot];
    const auto tex = textures.GetTexture(sprite);

    auto x = action.mX;
    auto y = action.mY;

    auto scale = glm::vec3{1,1,1};

    if (action.mTargetWidth != 0)
    {
        scale.x = static_cast<float>(action.mTargetWidth) / tex.GetWidth();
        scale.y = static_cast<float>(action.mTargetHeight) / tex.GetHeight();
    }

    if (action.mFlippedInY)
    {
        // Need to shift before flip to ensure sprite stays in same
        // relative pos. One way of achieving rotation about the 
        // centerline of the sprite...
        x += (static_cast<float>(tex.GetWidth()) * scale.x);
        scale.x *= -1;
    }
    
    return SceneSprite{
        sprite,
        glm::vec3{x, y, 0},
        scale};
}

}
