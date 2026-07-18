#include "bak/collision.hpp"

#include "bak/model.hpp"
#include "bak/worldFactory.hpp"
#include "graphics/glm.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>
#include <limits>

namespace BAK {

// Assumes the point is already in model clip space
bool PointInClipElement(glm::vec2 point, const ClipElement& element)
{
    constexpr float epsilon = 1e-5f;

    for (const auto& elemPoint : element.mPoints)
    {
        auto dir = point - glm::cast<float>(elemPoint.mXY);
        auto normal = glm::normalize(glm::cast<float>(elemPoint.mNormal));
        auto dot = glm::dot(dir, normal);
        if (dot > epsilon)
        {
            return false;
        }
    }

    return true;
}

bool PointInModelClip(glm::vec2 point, const ModelClip& clip)
{
    if (std::abs(point.x) > clip.mRadius.x || std::abs(point.y) > clip.mRadius.y)
    {
        return false;
    }

    for (const auto& elem : clip.mElements)
    {
        if (PointInClipElement(point, elem))
        {
            return true;
        }
    }

    return false;
}

bool BlocksMovement(const ZoneItem& item)
{
    const auto& clip = item.GetModelClip();
    const bool hasClip = clip && !clip->mElements.empty();
    const auto flags = item.GetEntityFlags();

    if (!hasClip) return false;

    if (flags == EF_TERRAIN)
    {
        switch (item.GetEntityType())
        {
        case EntityType::INTERIOR: return true;
        default: return false;
        }
    }

    if (item.GetEntityType() == EntityType::BRIDGE)
    {
        return false;
    }

    return true;
}

bool AllowsMovement(const ZoneItem& item)
{
    const auto& clip = item.GetModelClip();
    const bool hasClip = clip && !clip->mElements.empty();
    const auto flags = item.GetEntityFlags();

    if (!hasClip) return false;

    if (flags == EF_TERRAIN)
    {
        switch (item.GetEntityType())
        {
        case EntityType::EXTERIOR: return true;
        default: break;
        }
    }

    if (item.GetEntityType() == EntityType::BRIDGE
        || item.GetEntityType() == EntityType::TUNNEL1)
    {
        return true;
    }

    return false;
}

std::optional<float> ComputeHeight(
    glm::vec2 modelClipPos,
    const ClipElement& element)
{
    if (!element.mHeightPoint)
    {
        return std::nullopt;
    }

    const glm::vec2 heightDirection = glm::vec2(element.mHeightPoint->mNormal);
    const glm::vec2 referencePoint = glm::vec2(element.mHeightPoint->mXY);

    if (heightDirection == glm::vec2{0, 0})
    {
        return static_cast<float>(element.mBaseHeight);
    }

    const float slopeDistance = glm::dot(heightDirection, referencePoint - modelClipPos);
    const float height = static_cast<float>(element.mBaseHeight)
        + (slopeDistance * static_cast<float>(element.mScale)) / 4096.0f;

    return height;
}

std::optional<float> ComputeHeight(
    glm::vec2 modelClipPos,
    const ModelClip& clip)
{
    for (std::size_t i = 0; i < clip.mElements.size(); ++i)
    {
        if (PointInClipElement(modelClipPos, clip.mElements[i]))
        {
            auto height = ComputeHeight(modelClipPos, clip.mElements[i]);
            if (height)
            {
                return *height;
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}

float ComputeWorldHeight(
    float height,
    float itemScale)
{
    return BAK::gBakCameraHeight + height * itemScale;
}

glm::vec2 WorldToModelClipSpace(
    glm::vec2 playerPos,
    glm::vec2 itemPos,
    float rotation,
    float scale)
{
    auto local = playerPos - itemPos;
    auto rotated = (rotation != 0.0f)
        ? glm::rotate(local, -rotation)
        : local;
    auto modelSpace = rotated / scale;
    return modelSpace;
}

glm::vec2 ModelClipToWorldSpace(
    glm::vec2 playerPos,
    glm::vec2 itemPos,
    float rotation,
    float scale)
{
    auto scaled = playerPos * scale;

    auto rotated = (rotation != 0.0f)
        ? glm::rotate(scaled, rotation)
        : scaled;

    return itemPos + rotated;
}

}
