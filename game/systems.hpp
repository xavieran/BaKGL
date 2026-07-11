#pragma once

#include "bak/constants.hpp"
#include "bak/types.hpp"

#include "com/visit.hpp"

#include "graphics/glm.hpp"
#include "graphics/renderer.hpp"

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
        glm::vec3 scale,
        std::optional<glm::vec4>* instanceColor = nullptr);

    BAK::EntityIndex GetId() const;

    const glm::mat4& GetModelMatrix() const;
	const glm::vec3& GetLocation() const;

    std::pair<unsigned, unsigned> GetObject() const;
    std::optional<glm::vec4> GetInstanceColor() const
    {
        return mInstanceColor ? *mInstanceColor : std::nullopt;
    }

    void SetVisible(bool visible) { mVisible = visible; }
    bool GetVisible() const { return mVisible; }
    void SetObject(std::pair<unsigned, unsigned> object) { mObject = object; }
private:
    glm::mat4 CalculateModelMatrix();

    BAK::EntityIndex mItemId;
    std::pair<unsigned, unsigned> mObject;

    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    glm::mat4 mModelMatrix;

    const std::optional<glm::vec4>* mInstanceColor{nullptr};
    bool mVisible{true};
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
        std::pair<unsigned, unsigned>* object,
        glm::vec3* location,
        glm::mat4* modelMatrix,
        std::optional<glm::vec4>* instanceColor = nullptr);

    BAK::EntityIndex GetId() const;

    const glm::mat4& GetModelMatrix() const;
    glm::vec3 GetLocation() const;

    std::pair<unsigned, unsigned> GetObject() const;
    const Graphics::RenderData* GetRenderData() const;
    std::optional<glm::vec4> GetInstanceColor() const
    {
        return mInstanceColor ? *mInstanceColor : std::nullopt;
    }

    void SetVisible(bool visible) { mVisible = visible; }
    bool GetVisible() const { return mVisible; }
private:
    BAK::EntityIndex mItemId;
    const Graphics::RenderData* mRenderData;

    std::pair<unsigned, unsigned>* mObject;
    glm::vec3* mLocation;
    glm::mat4* mModelMatrix;
    std::optional<glm::vec4>* mInstanceColor{nullptr};
    bool mVisible{true};
};

class Systems
{
public:
    Systems();

    BAK::EntityIndex GetNextItemId();
    void AddIntersectable(const Intersectable& item);
    void AddClickable(const Clickable& item);
    void AddRenderable(const Renderable& item);
    void RemoveRenderable(BAK::EntityIndex);
    void EnableRenderable(BAK::EntityIndex id, bool);
    void SetRenderableFrame(BAK::EntityIndex id, std::pair<unsigned, unsigned> object);
    void AddDynamicRenderable(const DynamicRenderable& item);
    void RemoveDynamicRenderable(BAK::EntityIndex);
    void RemoveClickable(BAK::EntityIndex);
    void AddSprite(const Renderable& item);
    void EnableSprite(BAK::EntityIndex id, bool visible);
    std::vector<BAK::EntityIndex> RunIntersection(glm::vec3 cameraPos) const;

    BAK::EntityIndex AddTextRenderable(Graphics::TextRenderable r);
    Graphics::TextRenderable* GetTextRenderable(BAK::EntityIndex id);
    void RemoveTextRenderable(BAK::EntityIndex id);
    void ClearTextRenderables();
    const std::vector<Graphics::TextRenderable>& GetTextRenderables() const;

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
    std::vector<Graphics::TextRenderable> mTextRenderables;
};
