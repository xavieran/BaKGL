#pragma once

#include "bak/constants.hpp"

#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <optional>
#include <variant>
#include <vector>


class Intersectable
{
public:
    struct Rect
    {
        double mWidth;
        double mHeight;
    };

    struct Circle
    {
        double mRadius;
    };

    using IntersectionType = std::variant<Circle, Rect>;

    Intersectable(
        unsigned itemId,
        IntersectionType intersection,
        glm::vec3 location)
    :
        mItemId{itemId},
        mIntersection{intersection},
        mLocation{location}
    {}

    unsigned GetId() const { return mItemId; }
    bool Intersects(glm::vec3 position) const
    {
        return std::visit(
            overloaded{
                [&](const Rect& rect) -> bool
                {
                    const auto x_d = rect.mWidth / 2;
                    const auto z_d = rect.mHeight / 2;
                    return ((mLocation.x - x_d) < position.x)
                        && ((mLocation.x + x_d) > position.x)
                        && ((mLocation.z - z_d) < position.z)
                        && ((mLocation.z + z_d) > position.z);
                },
                [&](const Circle& circle)
                {

                    auto distance = glm::distance(mLocation, position);
                    return distance < circle.mRadius;
                }
            },
            mIntersection);
    }

    auto GetLocation() const { return mLocation; }

private:
    unsigned mItemId;
    IntersectionType mIntersection;
    glm::vec3 mLocation;
};

class Clickable
{
public:
    Clickable(
        unsigned itemId,
        double radius,
        glm::vec3 location)
    :
        mItemId{itemId},
        mRadius{radius},
        mLocation{location}
    {}

    unsigned GetId() const { return mItemId; }
    double GetRadius() const { return mRadius; }

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

    void AddClickable(const Clickable& item)
    {
        mClickables.emplace_back(item);
    }

    void AddRenderable(const Renderable& item)
    {
        mRenderables.emplace_back(item);
    }

    void AddSprite(const Renderable& item)
    {
        mSprites.emplace_back(item);
    }

    std::optional<unsigned> RunIntersection(glm::vec3 cameraPos) const
    {
        for (const auto& item : GetIntersectables())
        {
            if (item.Intersects(cameraPos))
                return item.GetId();
        }

        return std::optional<unsigned>{};
    }

    std::optional<unsigned> RunClickable(std::pair<glm::vec3, glm::vec3> line) const
    {
        double bestDistance = 1e9;
        auto bestId = std::optional<unsigned>{};
        for (const auto& clickable : GetClickables())
        {
            glm::vec3 p;
            glm::vec3 n;

            bool intersecting = glm::intersectLineSphere(
                line.first,
                line.second,
                clickable.GetLocation(),
                clickable.GetRadius(),
                p,
                n,
                p,
                n);

            if (intersecting)
            {
                auto distance = glm::distance(line.first, clickable.GetLocation());
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestId = clickable.GetId();
                }
            }
        }

        return bestId;
    }

    const std::vector<Intersectable>& GetIntersectables() const { return mIntersectables; }
    const std::vector<Renderable>& GetRenderables() const { return mRenderables; }
    const std::vector<Renderable>& GetSprites() const { return mSprites; }
    const std::vector<Clickable>& GetClickables() const { return mClickables; }

private:
    unsigned mNextItemId;

    std::vector<Intersectable> mIntersectables;
    std::vector<Renderable> mRenderables;
    std::vector<Renderable> mSprites;
    std::vector<Clickable> mClickables;
};
