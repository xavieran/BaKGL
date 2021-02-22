#pragma once

#include "glm.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

class Intersectable
{
public:
    Intersectable(
        unsigned itemId,
        double radius,
        glm::vec3 location)
    :
        mItemId{itemId},
        mRadius{radius},
        mLocation{location}
    {}

    unsigned GetId() const { return mItemId; }
    auto GetRadius() const { return mRadius; }
    auto GetLocation() const { return mLocation; }

private:
    unsigned mItemId;
    double mRadius;
    glm::vec3 mLocation;
};

class Renderable
{
public:
    Renderable(
        unsigned itemId,
        std::pair<unsigned, unsigned> object,
        glm::vec3 location,
        glm::vec3 rotation,
        glm::vec3 scale)
    :
        mItemId{itemId},
        mObject{object},
        mLocation{location},
        mRotation{rotation},
        mScale{scale},
        mModelMatrix{CalculateModelMatrix()}
    {}

    unsigned GetId() const { return mItemId; }

    const glm::mat4& GetModelMatrix() const
    {
        return mModelMatrix;
    }

	const auto& GetLocation() const { return mLocation; }

    std::pair<unsigned, unsigned> GetObject() const
    {
        return mObject;
    }

private:

    glm::mat4 CalculateModelMatrix()
    {
        auto modelMatrix = glm::mat4{1.0};
        modelMatrix = glm::translate(modelMatrix, mLocation / BAK::gWorldScale);
        modelMatrix = glm::scale(modelMatrix, mScale);
        // Dodgy... only works for rotation about y
        modelMatrix = glm::rotate(
            modelMatrix,
            mRotation.y,
            glm::vec3(0,1,0));

        return modelMatrix;
    }

    unsigned mItemId;
    std::pair<unsigned, unsigned> mObject;

    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    glm::mat4 mModelMatrix;
};

class Systems
{
public:

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

    // For now these are identical to intersectables in terms
    // of mechanics (bounding spheres) but are only checked when clicked
    void AddClickable(const Intersectable& item)
    {
        mClickables.emplace_back(item);
    }

    void AddRenderable(const Renderable& item)
    {
        mRenderables.emplace_back(item);
    }

    const std::vector<Intersectable>& GetIntersectables() const { return mIntersectables; }
    const std::vector<Renderable>& GetRenderables() const { return mRenderables; }
    const std::vector<Intersectable>& GetClickables() const { return mClickables; }

private:
    unsigned mNextItemId;

    std::vector<Intersectable> mIntersectables;
    std::vector<Renderable> mRenderables;
    std::vector<Intersectable> mClickables;
};
