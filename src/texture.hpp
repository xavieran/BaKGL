#pragma once

#include "xbak/Image.h"
#include "xbak/Palette.h"

namespace BAK {

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
    const TextureType& GetTexture() const { return mTexture; }

private:
    TextureType mTexture;
    unsigned mWidth;
    unsigned mHeight;
};

Texture ImageToTexture(const Image& image, const Palette& palette)
{
    auto texture = Texture::TextureType{};
    auto* pixels = image.GetPixels();

    for (int i = 0; i < (image.GetWidth() * image.GetHeight()); i++)
    {
        auto color = palette.GetColor(pixels[i]);
        // palette color 0 is transparency
        texture.push_back(
            BAK::ToGlColor<float>(color, pixels[i] == 0));
    }

    auto tex = Texture{
        texture,
        static_cast<unsigned>(image.GetWidth()),
        static_cast<unsigned>(image.GetHeight())};

    // For OpenGL
    tex.Invert();
    
    return tex;
}

}
