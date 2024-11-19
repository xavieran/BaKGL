#include "com/png.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "com/stb_image.h"

#include <stdexcept>
#include <string>

PNGImage LoadPNG(const char* filename)
{
    PNGImage image{};

    int x, y, n;
    static constexpr auto RGBA_SIZE = 4;
    std::uint8_t* data = stbi_load(filename, &x, &y, &n, RGBA_SIZE);
    if (!data)
    {
        throw std::runtime_error("Failed to load PNG: " + std::string{filename});
    }

    image.mWidth = x;
    image.mHeight = y;
    image.mPixels.resize(x * y);

    for (int i = 0; i < x * y; ++i)
    {
        image.mPixels[i].r = data[i * RGBA_SIZE];
        image.mPixels[i].g = data[i * RGBA_SIZE + 1];
        image.mPixels[i].b = data[i * RGBA_SIZE + 2];
        image.mPixels[i].a = data[i * RGBA_SIZE + 3];
    }

    stbi_image_free(data);
    return image;
}
