#include "graphics/sprites.hpp"

#include "com/assert.hpp"

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
    mNonSpriteObjects{other.mNonSpriteObjects},
    mVertexArray{std::move(other.mVertexArray)},
    mBuffers{std::move(other.mBuffers)},
    mTextureBuffer{std::move(other.mTextureBuffer)},
    mObjects{std::move(other.mObjects)}
{
}

Sprites& Sprites::operator=(Sprites&& other) noexcept
{
    this->mVertexArray = std::move(other.mVertexArray);
    this->mBuffers = std::move(other.mBuffers);
    this->mTextureBuffer = std::move(other.mTextureBuffer);
    this->mObjects = other.mObjects;
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

    mBuffers.AddBuffer("vertex", 0, 3);
    mBuffers.AddBuffer("textureCoord", 1, 3);

    mBuffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, mObjects.mVertices);
    mBuffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, mObjects.mTextureCoords);
    mBuffers.LoadBufferDataGL(mBuffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, mObjects.mIndices);
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

SpriteManager::SpriteManager()
:
    mSprites(),
    mNextSpriteSheet{0},
    mActiveSpriteSheet{}
{
    // Seems excessive...
    mSprites.reserve(128);
}

SpriteSheetIndex SpriteManager::AddSpriteSheet()
{
    const auto& logger = Logging::LogState::GetLogger("SpriteManager");
    const auto spriteSheetIndex = NextSpriteSheet();
    logger.Debug() << "Adding sprite sheet index: " << spriteSheetIndex << "\n";
    ASSERT(mSprites.size() == spriteSheetIndex.mValue);
    mSprites.emplace_back();
    return spriteSheetIndex;
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
    ASSERT(mSprites.size() > spriteSheet.mValue);
    return mSprites[spriteSheet.mValue];
}

SpriteSheetIndex SpriteManager::NextSpriteSheet()
{
    return Graphics::SpriteSheetIndex{mNextSpriteSheet++};
}

}
