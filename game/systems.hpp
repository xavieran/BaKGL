#pragma once

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

namespace Graphics {
class RenderData;
}

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
        BAK::EntityIndex itemId,
        IntersectionType intersection,
        glm::vec3 location);

    BAK::EntityIndex GetId() const;
    bool Intersects(glm::vec3 position) const;
    glm::vec3 GetLocation() const;

private:
    BAK::EntityIndex mItemId;
    IntersectionType mIntersection;
    glm::vec3 mLocation;
};

class Clickable
{
public:
    explicit Clickable(
        BAK::EntityIndex itemId);

    BAK::EntityIndex GetId() const;

private:
    BAK::EntityIndex mItemId;
};

class Renderable
{
public:
    Renderable(
        BAK::EntityIndex itemId,
        std::pair<unsigned, unsigned> object,
        glm::vec3 location,
        glm::vec3 rotation,
        glm::vec3 scale);

    BAK::EntityIndex GetId() const;

    const glm::mat4& GetModelMatrix() const;
	const glm::vec3& GetLocation() const;

    std::pair<unsigned, unsigned> GetObject() const;
private:
    glm::mat4 CalculateModelMatrix();

    BAK::EntityIndex mItemId;
    std::pair<unsigned, unsigned> mObject;

    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    glm::mat4 mModelMatrix;
};

class Tickable
{
public:
    Tickable(
        BAK::EntityIndex itemId);

    BAK::EntityIndex GetId() const;

private:
    BAK::EntityIndex mItemId;
};

class DynamicRenderable
{
public:
    DynamicRenderable(
        BAK::EntityIndex itemId,
        const Graphics::RenderData* renderData,
        std::pair<unsigned, unsigned> object,
        glm::vec3 location,
        glm::vec3 rotation,
        glm::vec3 scale);

    BAK::EntityIndex GetId() const;

    const glm::mat4& GetModelMatrix() const;
	const glm::vec3& GetLocation() const;

    std::pair<unsigned, unsigned> GetObject() const;
    const Graphics::RenderData* GetRenderData() const;
private:
    glm::mat4 CalculateModelMatrix();

    BAK::EntityIndex mItemId;
    std::pair<unsigned, unsigned> mObject;
    const Graphics::RenderData* mRenderData;

    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    glm::mat4 mModelMatrix;
};


class Systems
{
public:
    Systems();

    BAK::EntityIndex GetNextItemId();
    void AddIntersectable(const Intersectable& item);
    void AddClickable(const Clickable& item);
    void AddRenderable(const Renderable& item);
    void AddDynamicRenderable(const DynamicRenderable& item);
    void RemoveRenderable(BAK::EntityIndex i);
    void AddSprite(const Renderable& item);
    std::vector<BAK::EntityIndex> RunIntersection(glm::vec3 cameraPos) const;

    const std::vector<Intersectable>& GetIntersectables() const;
    const std::vector<Renderable>& GetRenderables() const;
    const std::vector<DynamicRenderable>& GetDynamicRenderables() const;
    const std::vector<Renderable>& GetSprites() const;
    const std::vector<Clickable>& GetClickables() const;

private:
    unsigned mNextItemId;

    std::vector<Intersectable> mIntersectables;
    std::vector<Renderable> mRenderables;
    std::vector<Renderable> mSprites;
    std::vector<DynamicRenderable> mDynamicRenderables;
    std::vector<Clickable> mClickables;
};
