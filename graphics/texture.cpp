#include "graphics/texture.hpp"

#include "com/assert.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace Graphics {

Texture::Texture(
    const TextureType& texture,
    unsigned width,
    unsigned height,
    unsigned targetWidth,
    unsigned targetHeight)
:
    mTexture{texture},
    mWidth{width},
    mHeight{height},
    mTargetWidth{targetWidth},
    mTargetHeight{targetHeight}
{}

Texture::Texture(
    unsigned width,
    unsigned height,
    unsigned targetWidth,
    unsigned targetHeight)
:
    mTexture{width * height, glm::vec4{0}},
    mWidth{width},
    mHeight{height},
    mTargetWidth{targetWidth},
    mTargetHeight{targetHeight}
{
}

// Get pixel wrapping access
glm::vec4 Texture::GetPixel(unsigned x, unsigned y) const
{
    return mTexture[
        (x % GetWidth()) 
        + (y % GetHeight()) * GetWidth()];
}

void Texture::SetPixel(unsigned x, unsigned y, glm::vec4 color)
{
    mTexture[
        (x % GetWidth()) 
        + (y % GetHeight()) * GetWidth()] = color;
}

void Texture::Invert()
{
    for (unsigned x = 0; x < GetWidth(); x++)
        for (unsigned y = 0; y < (GetHeight() / 2); y++)
            std::swap(
                mTexture[x + (y * GetWidth())],
                mTexture[x + ((GetHeight() - 1 - y) * GetWidth())]);
}
Texture Texture::GetRegion(glm::ivec2 pos, glm::uvec2 dims) const
{
    // FIX TARGET
    auto newTexture = Texture{dims.x, dims.y, dims.x, dims.y};
    for (unsigned x = 0; x < dims.x; x++)
    {
        for (unsigned y = 0; y < dims.y; y++)
        {
            newTexture.SetPixel(x, y, GetPixel(pos.x + x, pos.y + y));
        }
    }
    return newTexture;
}

unsigned Texture::GetWidth() const { return mWidth; }
unsigned Texture::GetHeight() const { return mHeight; }
unsigned Texture::GetTargetWidth() const { return mTargetWidth; }
unsigned Texture::GetTargetHeight() const { return mTargetHeight; }
glm::ivec2 Texture::GetDims() const { return glm::ivec2(GetTargetWidth(), GetTargetHeight()); }
const Texture::TextureType& Texture::GetTexture() const { return mTexture; }
Texture::TextureType& Texture::GetTexture() { return mTexture; }

TextureStore::TextureStore()
:
    mTextures{},
    mMaxHeight{0},
    mMaxWidth{0},
    mMaxDim{0}
{}

void TextureStore::AddTexture(const Texture& texture)
{
    if (texture.GetHeight() > mMaxHeight)
        mMaxHeight = texture.GetHeight();
    if (texture.GetWidth() > mMaxWidth)
        mMaxWidth = texture.GetWidth();
    mMaxDim = std::max(mMaxHeight, mMaxWidth);
    mTextures.emplace_back(texture);
}

const std::vector<Texture>& TextureStore::GetTextures() const { return mTextures; }
const Texture& TextureStore::GetTexture(std::size_t i) const { ASSERT(i < mTextures.size()); return mTextures[i]; }
Texture& TextureStore::GetTexture(std::size_t i) { ASSERT(i < mTextures.size()); return mTextures[i]; }

unsigned TextureStore::GetMaxDim() const { return mMaxDim; }
unsigned TextureStore::GetMaxHeight() const { return mMaxHeight; }
unsigned TextureStore::GetMaxWidth() const { return mMaxWidth; }
std::size_t TextureStore::size() const { return mTextures.size(); }

}
