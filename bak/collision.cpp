#include "bak/collision.hpp"

#include "bak/model.hpp"
#include "graphics/glm.hpp"

#include <glm/geometric.hpp>

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
            if (dot < epsilon)
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

}
