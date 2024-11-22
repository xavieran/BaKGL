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
        unsigned height,
        unsigned targetWidth,
        unsigned targetHeight);

    Texture(
        unsigned width,
        unsigned height,
        unsigned targetWidth,
        unsigned targetHeight);

    // Get pixel wrapping access
    glm::vec4 GetPixel(unsigned x, unsigned y) const;
    void SetPixel(unsigned x, unsigned y, glm::vec4 color);
    void Invert();

    Texture GetRegion(glm::ivec2 pos, glm::uvec2 dims) const;

    unsigned GetWidth() const;
    unsigned GetHeight() const;
    unsigned GetTargetWidth() const;
    unsigned GetTargetHeight() const;
    glm::ivec2 GetDims() const;
    const TextureType& GetTexture() const;
    TextureType& GetTexture();

private:
    TextureType mTexture;
    unsigned mWidth;
    unsigned mHeight;
    unsigned mTargetWidth;
    unsigned mTargetHeight;
};

class TextureStore
{
public:
    TextureStore();

    void AddTexture(const Texture& texture);

    const std::vector<Texture>& GetTextures() const;
    const Texture& GetTexture(std::size_t i) const;
    Texture& GetTexture(std::size_t i);

    unsigned GetMaxDim() const;
    unsigned GetMaxHeight() const;
    unsigned GetMaxWidth() const;
    std::size_t size() const;

private:
    std::vector<Texture> mTextures;

    unsigned mMaxHeight;
    unsigned mMaxWidth;
    unsigned mMaxDim;
};

}
