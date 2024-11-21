#pragma once

#include "graphics/quad.hpp"
#include "graphics/opengl.hpp"
#include "graphics/texture.hpp"
#include "graphics/types.hpp"

#include <GL/glew.h>

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Graphics {

class Sprites
{
public:
    Sprites() noexcept;

    Sprites(const Sprites&) = delete;
    Sprites& operator=(const Sprites&) = delete;

    Sprites(Sprites&& other) noexcept;
    Sprites& operator=(Sprites&& other) noexcept;

    void BindGL() const;
    void UnbindGL() const;
    
    void LoadTexturesGL(const TextureStore& textures);

    std::size_t size();

    auto GetRect() const
    {
        return mObjects.GetObject(0);
    }

    auto Get(unsigned i) const
    {
        return mObjects.GetObject(i + mNonSpriteObjects);
    }

    glm::vec2 GetDimensions(unsigned i) const;
    
//private:
    std::size_t mNonSpriteObjects;
    VertexArrayObject mVertexArray;
    GLBuffers mBuffers;
    TextureBuffer mTextureBuffer;
    QuadStorage mObjects;
    std::vector<glm::vec2> mSpriteDimensions;
};

class SpriteManager;

struct TemporarySpriteHandle
{
    SpriteManager* mManager;
    SpriteSheetIndex mSpriteSheet;
};

struct DestroySpriteSheet
{
    void operator()(TemporarySpriteHandle* handle);
};

class SpriteManager
{
public:
    SpriteManager();

    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;

    SpriteManager(SpriteManager&& other) = delete;
    SpriteManager& operator=(SpriteManager&& other) = delete;

    using TemporarySpriteSheet = std::unique_ptr<TemporarySpriteHandle,  DestroySpriteSheet>;
    SpriteSheetIndex AddSpriteSheet();
    TemporarySpriteSheet AddTemporarySpriteSheet();
    void RemoveSpriteSheet(SpriteSheetIndex);

    void DeactivateSpriteSheet();
    void ActivateSpriteSheet(SpriteSheetIndex spriteSheet);

    Sprites& GetSpriteSheet(SpriteSheetIndex spriteSheet);

private:
    SpriteSheetIndex NextSpriteSheet();

    std::unordered_map<SpriteSheetIndex, Sprites> mSprites;

    unsigned mNextSpriteSheet;
    std::optional<SpriteSheetIndex> mActiveSpriteSheet;
};

}
