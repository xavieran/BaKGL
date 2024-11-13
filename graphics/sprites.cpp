#include "graphics/sprites.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <GL/glew.h>

#include <memory>
#include <optional>
#include <vector>

namespace Graphics {

Sprites::Sprites() noexcept
:
    mNonSpriteObjects{1},
    mVertexArray{},
    mBuffers{},
    mTextureBuffer{GL_TEXTURE_2D_ARRAY},
    mObjects{},
    mSpriteDimensions{}
{
}

Sprites::Sprites(Sprites&& other) noexcept
:
    mTextureBuffer{GL_TEXTURE_2D_ARRAY}
{
    (*this) = std::move(other);
}

Sprites& Sprites::operator=(Sprites&& other) noexcept
{
    if (this == &other) return *this;
    this->mNonSpriteObjects = std::move(other.mNonSpriteObjects);
    this->mVertexArray = std::move(other.mVertexArray);
    this->mBuffers = std::move(other.mBuffers);
    this->mTextureBuffer = std::move(other.mTextureBuffer);
    this->mObjects = other.mObjects;
    this->mSpriteDimensions = other.mSpriteDimensions;
    return *this;
}

void Sprites::BindGL() const
{
    mVertexArray.BindGL();

    // FIXME!!! blergh...
    glActiveTexture(GL_TEXTURE0);
    mTextureBuffer.BindGL();
}

void Sprites::UnbindGL() const
{
    mVertexArray.UnbindGL();
    mTextureBuffer.UnbindGL();
}

void Sprites::LoadTexturesGL(const TextureStore& textures)
{
    mTextureBuffer.LoadTexturesGL(
        textures.GetTextures(),
        textures.GetMaxDim());

    // Normal quad for use as arbitrary rectangle
    mObjects.AddObject(Quad{1.0, 1.0, 1.0, 0});
    // This is why mNonSpriteObjects = 1;

    for (unsigned i = 0; i < textures.GetTextures().size(); i++)
    {
        const auto& tex = textures.GetTexture(i);
        mObjects.AddObject(
            Quad{
                static_cast<double>(tex.GetWidth()),
                static_cast<double>(tex.GetHeight()),
                static_cast<double>(textures.GetMaxDim()),
                i});
        mSpriteDimensions.emplace_back(
            tex.GetWidth(),
            tex.GetHeight());
    }

    mVertexArray.BindGL();

    mBuffers.AddStaticArrayBuffer<glm::vec3>("vertex", GLLocation{0});
    mBuffers.AddStaticArrayBuffer<glm::vec3>("textureCoord", GLLocation{1});
    mBuffers.AddElementBuffer("elements");

    mBuffers.LoadBufferDataGL("vertex", mObjects.mVertices);
    mBuffers.LoadBufferDataGL("textureCoord", mObjects.mTextureCoords);
    mBuffers.LoadBufferDataGL("elements", mObjects.mIndices);
    mBuffers.BindArraysGL();
    
    UnbindGL();
}

std::size_t Sprites::size()
{
    return mSpriteDimensions.size();
}

glm::vec2 Sprites::GetDimensions(unsigned i) const
{
    ASSERT(i < mSpriteDimensions.size());
    return mSpriteDimensions[i];
}

void DestroySpriteSheet::operator()(TemporarySpriteHandle* handle)
{
    assert(handle->mManager);
    handle->mManager->RemoveSpriteSheet(handle->mSpriteSheet);
    delete handle;
}

SpriteManager::SpriteManager()
:
    mSprites(),
    mNextSpriteSheet{0},
    mActiveSpriteSheet{}
{
}

SpriteSheetIndex SpriteManager::AddSpriteSheet()
{
    const auto& logger = Logging::LogState::GetLogger("SpriteManager");
    const auto spriteSheetIndex = NextSpriteSheet();
    logger.Debug() << "Adding sprite sheet index: " << spriteSheetIndex << "\n";
    mSprites.emplace(spriteSheetIndex, Sprites{});
    return spriteSheetIndex;
}

SpriteManager::TemporarySpriteSheet SpriteManager::AddTemporarySpriteSheet()
{
    return TemporarySpriteSheet(new TemporarySpriteHandle{this, AddSpriteSheet()});
}

void SpriteManager::RemoveSpriteSheet(SpriteSheetIndex index)
{
    const auto& logger = Logging::LogState::GetLogger("SpriteManager");
    logger.Debug() << "Removing sprite sheet index: " << index << std::endl;
    DeactivateSpriteSheet();
    mSprites.erase(index);
}

void SpriteManager::DeactivateSpriteSheet()
{
    if (mActiveSpriteSheet)
    {
        GetSpriteSheet(*mActiveSpriteSheet).UnbindGL();
        mActiveSpriteSheet = std::optional<SpriteSheetIndex>{};
    }
}

void SpriteManager::ActivateSpriteSheet(SpriteSheetIndex spriteSheet)
{
    if (!mActiveSpriteSheet || spriteSheet != *mActiveSpriteSheet)
    {
        GetSpriteSheet(spriteSheet).BindGL();
        mActiveSpriteSheet = spriteSheet;
    }
}

Sprites& SpriteManager::GetSpriteSheet(SpriteSheetIndex spriteSheet)
{
    ASSERT(mSprites.contains(spriteSheet));
    return mSprites.find(spriteSheet)->second;
}

SpriteSheetIndex SpriteManager::NextSpriteSheet()
{
    return Graphics::SpriteSheetIndex{mNextSpriteSheet++};
}

}
