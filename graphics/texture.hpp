#pragma once

#include "com/assert.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace Graphics {

class Texture
{
public:
    using TextureType = std::vector<glm::vec4>;

    Texture(
        const TextureType& texture,
        unsigned width,
        unsigned height)
    :
        mTexture{texture},
        mWidth{width},
        mHeight{height}
    {}

    Texture(
        unsigned width,
        unsigned height)
    :
        mTexture{width * height, glm::vec4{0}},
        mWidth{width},
        mHeight{height}
    {
    }

    // Get pixel wrapping access
    const auto& GetPixel(unsigned x, unsigned y) const
    {
        return mTexture[
            (x % GetWidth()) 
            + (y % GetHeight()) * GetWidth()];
    }

    void SetPixel(unsigned x, unsigned y, glm::vec4 color)
    {
        mTexture[
            (x % GetWidth()) 
            + (y % GetHeight()) * GetWidth()] = color;
    }

    void Invert()
    {
        for (unsigned x = 0; x < GetWidth(); x++)
            for (unsigned y = 0; y < (GetHeight() / 2); y++)
                std::swap(
                    mTexture[x + (y * GetWidth())],
                    mTexture[x + ((GetHeight() - 1 - y) * GetWidth())]);
    }

    Texture GetRegion(glm::ivec2 pos, glm::uvec2 dims) const
    {
        auto newTexture = Texture{dims.x, dims.y};
        for (unsigned x = 0; x < dims.x; x++)
        {
            for (unsigned y = 0; y < dims.y; y++)
            {
                newTexture.SetPixel(x, y, GetPixel(pos.x + x, pos.y + y));
            }
        }
        return newTexture;
    }

    unsigned GetWidth() const { return mWidth; }
    unsigned GetHeight() const { return mHeight; }
    glm::ivec2 GetDims() const { return glm::ivec2(GetWidth(), GetHeight()); }
    const TextureType& GetTexture() const { return mTexture; }
    TextureType& GetTexture() { return mTexture; }

private:
    TextureType mTexture;
    unsigned mWidth;
    unsigned mHeight;
};

class TextureStore
{
public:
    TextureStore()
    :
        mTextures{},
        mMaxHeight{0},
        mMaxWidth{0},
        mMaxDim{0}
    {}

    void AddTexture(const Texture& texture)
    {
        if (texture.GetHeight() > mMaxHeight)
            mMaxHeight = texture.GetHeight();
        if (texture.GetWidth() > mMaxWidth)
            mMaxWidth = texture.GetWidth();
        mMaxDim = std::max(mMaxHeight, mMaxWidth);
        mTextures.emplace_back(texture);
    }

    const std::vector<Texture>& GetTextures() const { return mTextures; }
    const Texture& GetTexture(std::size_t i) const { ASSERT(i < mTextures.size()); return mTextures[i]; }
    Texture& GetTexture(std::size_t i) { ASSERT(i < mTextures.size()); return mTextures[i]; }

    unsigned GetMaxDim() const { return mMaxDim; }
    unsigned GetMaxHeight() const { return mMaxHeight; }
    unsigned GetMaxWidth() const { return mMaxWidth; }
    std::size_t size() const { return mTextures.size(); }

private:
    std::vector<Texture> mTextures;

    unsigned mMaxHeight;
    unsigned mMaxWidth;
    unsigned mMaxDim;

};

}
