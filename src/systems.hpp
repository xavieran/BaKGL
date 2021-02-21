#pragma once

#include "glm.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

class Intersectable
{
    Intersectable(
        unsigned itemId,
        double radius,
        glm::vec3 location)
    :
        mItemId{itemId},
        mRadius{radius},
        mLocation{location}
    {}

private:
    unsigned mItemId;
    double mRadius;
    glm::vec3 mLocation;
};

class Systems
{
    Systems()
    :
        mNextItemId{0}
    {}

    unsigned GetNextItemId()
    {
        return mNextItemId++;
    }

    void AddIntersectable(const Intersectable& item)
    {
        mIntersectables.emplace_back(item);
    }

    const std::vector<Intersectable>& GetIntersectables() const { return mIntersectables; }
private:
    unsigned mNextItemId;

    std::vector<Intersectable> mIntersectables;
};
