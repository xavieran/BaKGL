#include "game/systems.hpp"

#include "bak/constants.hpp"
#include "bak/types.hpp"

#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <optional>
#include <variant>
#include <vector>


Intersectable::Intersectable(
    BAK::EntityIndex itemId,
    IntersectionType intersection,
    glm::vec3 location)
:
    mItemId{itemId},
    mIntersection{intersection},
    mLocation{location}
{}

BAK::EntityIndex Intersectable::GetId() const { return mItemId; }
bool Intersectable::Intersects(glm::vec3 position) const
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

glm::vec3 Intersectable::GetLocation() const { return mLocation; }

Clickable::Clickable(
    BAK::EntityIndex itemId)
:
    mItemId{itemId}
{}

BAK::EntityIndex Clickable::GetId() const { return mItemId; }

Renderable::Renderable(
    BAK::EntityIndex itemId,
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

BAK::EntityIndex Renderable::GetId() const { return mItemId; }

const glm::mat4& Renderable::GetModelMatrix() const
{
    return mModelMatrix;
}

const glm::vec3& Renderable::GetLocation() const { return mLocation; }

std::pair<unsigned, unsigned> Renderable::GetObject() const
{
    return mObject;
}

glm::mat4 Renderable::CalculateModelMatrix()
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

DynamicRenderable::DynamicRenderable(
    BAK::EntityIndex itemId,
    const Graphics::RenderData* renderData,
    std::pair<unsigned, unsigned> object,
    glm::vec3 location,
    glm::vec3 rotation,
    glm::vec3 scale)
:
    mItemId{itemId},
    mObject{object},
    mRenderData{renderData},
    mLocation{location},
    mRotation{rotation},
    mScale{scale},
    mModelMatrix{CalculateModelMatrix()}
{}

BAK::EntityIndex DynamicRenderable::GetId() const { return mItemId; }

const glm::mat4& DynamicRenderable::GetModelMatrix() const
{
    return mModelMatrix;
}

const glm::vec3& DynamicRenderable::GetLocation() const { return mLocation; }

std::pair<unsigned, unsigned> DynamicRenderable::GetObject() const
{
    return mObject;
}

const Graphics::RenderData* DynamicRenderable::GetRenderData() const
{
    return mRenderData;
}

glm::mat4 DynamicRenderable::CalculateModelMatrix()
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


Systems::Systems()
:
    mNextItemId{0}
{}

BAK::EntityIndex Systems::GetNextItemId()
{
    return BAK::EntityIndex{mNextItemId++};
}

void Systems::AddIntersectable(const Intersectable& item)
{
    mIntersectables.emplace_back(item);
}

void Systems::AddClickable(const Clickable& item)
{
    mClickables.emplace_back(item);
}

void Systems::AddRenderable(const Renderable& item)
{
    mRenderables.emplace_back(item);
}

void Systems::AddDynamicRenderable(const DynamicRenderable& item)
{
    mDynamicRenderables.emplace_back(item);
}

void Systems::RemoveRenderable(BAK::EntityIndex i)
{
    auto it = std::find_if(
        mRenderables.begin(), mRenderables.end(),
        [i=i](const auto& r){ return r.GetId() == i; });
    if (it != mRenderables.end())
        mRenderables.erase(it);
}

void Systems::AddSprite(const Renderable& item)
{
    mSprites.emplace_back(item);
}

std::vector<BAK::EntityIndex> Systems::RunIntersection(glm::vec3 cameraPos) const
{
    auto result = std::vector<BAK::EntityIndex>{};
    for (const auto& item : GetIntersectables())
    {
        if (item.Intersects(cameraPos))
            result.emplace_back(item.GetId());
    }

    return result;
}

const std::vector<Intersectable>& Systems::GetIntersectables() const { return mIntersectables; }
const std::vector<Renderable>& Systems::GetRenderables() const { return mRenderables; }
const std::vector<DynamicRenderable>& Systems::GetDynamicRenderables() const { return mDynamicRenderables; }
const std::vector<Renderable>& Systems::GetSprites() const { return mSprites; }
const std::vector<Clickable>& Systems::GetClickables() const { return mClickables; }

