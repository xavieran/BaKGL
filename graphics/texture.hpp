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
    
    // Get pixel wrapping access
    const auto& GetPixel(unsigned x, unsigned y) const
    {
        return mTexture[
            (x % GetWidth()) 
            + (y % GetHeight()) * GetWidth()];
    }

    void Invert()
    {
        for (unsigned x = 0; x < GetWidth(); x++)
            for (unsigned y = 0; y < (GetHeight() / 2); y++)
                std::swap(
                    mTexture[x + (y * GetWidth())],
                    mTexture[x + ((GetHeight() - 1 - y) * GetWidth())]);
    }

    unsigned GetWidth() const { return mWidth; }
    unsigned GetHeight() const { return mHeight; }
    std::pair<unsigned, unsigned> GetDims() const
    {
        return std::make_pair(GetWidth(), GetHeight());
    }

    const TextureType& GetTexture() const { return mTexture; }

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
