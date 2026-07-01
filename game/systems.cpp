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
#include <utility>
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
    return Graphics::CalculateModelMatrix(mLocation, mScale, mRotation, BAK::gWorldScale);
}

DynamicRenderable::DynamicRenderable(
    BAK::EntityIndex itemId,
    const Graphics::RenderData* renderData,
    std::pair<unsigned, unsigned>* object,
    glm::vec3* location,
    glm::mat4* modelMatrix)
:
    mItemId{itemId},
    mRenderData{renderData},
    mObject{object},
    mLocation{location},
    mModelMatrix{modelMatrix}
{}

BAK::EntityIndex DynamicRenderable::GetId() const { return mItemId; }

const glm::mat4& DynamicRenderable::GetModelMatrix() const
{
    assert(mModelMatrix);
    return *mModelMatrix;
}

glm::vec3 DynamicRenderable::GetLocation() const
{
    assert(mLocation);
    return *mLocation;
}

std::pair<unsigned, unsigned> DynamicRenderable::GetObject() const
{
    assert(mObject);
    return *mObject;
}

const Graphics::RenderData* DynamicRenderable::GetRenderData() const
{
    assert(mRenderData);
    return mRenderData;
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

void Systems::RemoveDynamicRenderable(BAK::EntityIndex i)
{
    auto it = std::find_if(
        mDynamicRenderables.begin(), mDynamicRenderables.end(),
        [i=i](const auto& r){ return r.GetId() == i; });
    if (it != mDynamicRenderables.end())
    {
        mDynamicRenderables.erase(it);
    }
}

void Systems::RemoveClickable(BAK::EntityIndex i)
{
    auto it = std::find_if(
        mClickables.begin(), mClickables.end(),
        [i=i](const auto& r){ return r.GetId() == i; });
    if (it != mClickables.end())
        mClickables.erase(it);
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

BAK::EntityIndex Systems::AddTextRenderable(Graphics::TextRenderable r)
{
    auto id = GetNextItemId();
    r.mEntityId = id;
    mTextRenderables.push_back(std::move(r));
    return id;
}

Graphics::TextRenderable& Systems::GetTextRenderable(BAK::EntityIndex id)
{
    for (auto& r : mTextRenderables)
    {
        if (r.mEntityId == id)
        {
            return r;
        }
    }
    assert(false);
    std::unreachable();
}

void Systems::RemoveTextRenderable(BAK::EntityIndex id)
{
    for (auto it = mTextRenderables.begin(); it != mTextRenderables.end(); ++it)
    {
        if (it->mEntityId == id)
        {
            mTextRenderables.erase(it);
            return;
        }
    }
}

void Systems::ClearTextRenderables()
{
    mTextRenderables.clear();
}

const std::vector<Graphics::TextRenderable>& Systems::GetTextRenderables() const
{
    return mTextRenderables;
}

const std::vector<Intersectable>& Systems::GetIntersectables() const { return mIntersectables; }
const std::vector<Renderable>& Systems::GetRenderables() const { return mRenderables; }
const std::vector<DynamicRenderable>& Systems::GetDynamicRenderables() const { return mDynamicRenderables; }
const std::vector<Renderable>& Systems::GetSprites() const { return mSprites; }
const std::vector<Clickable>& Systems::GetClickables() const { return mClickables; }

