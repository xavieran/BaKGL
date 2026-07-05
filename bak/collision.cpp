#include "bak/collision.hpp"

#include "bak/model.hpp"
#include "bak/worldFactory.hpp"
#include "graphics/glm.hpp"

#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>

namespace BAK {

// Assumes the point is already in model clip space
bool PointInModelClip(glm::vec2 point, const ModelClip& clip)
{
    constexpr float epsilon = 1e-5f;

    if (std::abs(point.x) > clip.mRadius.x || std::abs(point.y) > clip.mRadius.y)
    {
        return false;
    }

    bool anyWithin = false;
    for (const auto& elem : clip.mElements)
    {
        bool allWithin = true;
        for (const auto& elemPoint: elem.mPoints)
        {
            auto dir = point - glm::cast<float>(elemPoint.mXY);
            // TODO: Normalise it in intermediate object...
            auto normal = glm::normalize(glm::cast<float>(elemPoint.mNormal));
            auto dot = glm::dot(dir, normal);
            if (dot > epsilon)
            {
                allWithin = false;
            }
        }

        if (allWithin)
        {
            return true;
        }
    }

    return anyWithin;
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
        default: return false;
        }
    }

    if (item.GetEntityType() == EntityType::BRIDGE
        || item.GetEntityType() == EntityType::TUNNEL1)
    {
        return true;
    }

    return false;
}

glm::vec2 WorldToModelClipSpace(glm::vec2 playerPos, glm::vec2 itemPos, float rotation, float scale)
{
    auto local = playerPos - itemPos;
    auto rotated = (rotation != 0.0f)
        ? glm::rotate(local, -rotation)
        : local;
    auto modelSpace = rotated / scale;
    return modelSpace;
}

}
