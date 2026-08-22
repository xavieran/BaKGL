#pragma once

#include "bak/coordinates.hpp"
#include "bak/scene/sceneData.hpp"

#include <glm/glm.hpp>

#include <cmath>
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
    glm::vec2 mPosition;
    glm::vec2 mScale;
    float mRotation{0};
};

struct SceneRect
{
    glm::vec4 mColor;
    glm::vec2 mPosition;
    glm::vec2 mDimensions;
};

using DrawingAction = std::variant<
    EnableClipRegion,
    DisableClipRegion,
    SceneSprite,
    SceneRect>;

DrawingAction ConvertScriptAction(const BAK::ScriptAction& action);
EnableClipRegion ConvertScriptAction(const BAK::ClipRegion&);
DisableClipRegion ConvertScriptAction(const BAK::DisableClipRegion&);

template <typename T, typename S>
SceneSprite ConvertScriptAction(
    const BAK::DrawSprite& action,
    const T& textures,
    const S& offsets) // make this const
{
    const auto sprite = action.mSpriteIndex 
        + offsets.at(action.mImageSlot);
    const auto tex = textures.GetTexture(sprite);

    auto x = action.mX;
    auto y = action.mY;

    auto scale = glm::vec2{tex.GetTargetWidth(), tex.GetTargetHeight()};

    if (action.mTargetWidth != 0)
    {
        scale.x = static_cast<float>(action.mTargetWidth);
        scale.y = static_cast<float>(action.mTargetHeight);
    }

    if (action.mFlipX)
    {
        x += scale.x;
        scale.x *= -1;
    }

    if (action.mFlipY)
    {
        y += scale.y;
        scale.y *= -1;
    }


    return SceneSprite{
        sprite,
        glm::vec2{x, y},
        scale};
}

template <typename S>
SceneSprite ConvertScriptAction(
    const BAK::DrawSpriteRotated& action,
    const S& offsets)
{
    const auto sprite = action.mSpriteIndex
        + offsets.at(action.mImageSlot);

    const auto scale = glm::vec2{
        std::abs(action.mTargetWidth),
        std::abs(action.mTargetHeight)};

    const auto angle = BAK::ToRadians(action.mAngle);
    const auto widthAxis  = glm::vec2{std::cos(angle), std::sin(angle)} * scale.x;
    const auto heightAxis = glm::vec2{-std::sin(angle), std::cos(angle)} * scale.y;
    const auto position = glm::vec2{action.mX, action.mY}
        - (widthAxis + heightAxis) * 0.5f;

    // I should make the angle consistent, gl gui renderer
    // requires -ve angle which may be causing flipping
    // when rotating (e.g. look at Rainbow Parrot Inn lute)
    return SceneSprite{
        static_cast<unsigned>(sprite),
        position,
        scale,
        -angle};
}

template <typename T>
SceneSprite ConvertScriptAction(
    const BAK::DrawSprite& action,
    const T& textures)
{
    const auto sprite = static_cast<unsigned>(action.mSpriteIndex);
    const auto tex = textures.GetTexture(sprite);

    auto x = action.mX;
    auto y = action.mY;

    auto scale = glm::vec2{tex.GetTargetWidth(), tex.GetTargetHeight()};

    if (action.mTargetWidth != 0)
    {
        scale.x = static_cast<float>(action.mTargetWidth);
        scale.y = static_cast<float>(action.mTargetHeight);
    }

    if (action.mFlipX)
    {
        x += scale.x;
        scale.x *= -1;
    }

    if (action.mFlipY)
    {
        y += scale.y;
        scale.y *= -1;
    }


    return SceneSprite{
        sprite,
        glm::vec2{x, y},
        scale};
}
}
