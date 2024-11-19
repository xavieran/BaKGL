#include "com/png.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "com/stb_image_write.h"
#pragma GCC diagnostic pop

#include <stdexcept>

void WritePNG(const char* filename, const PNGImage& image)
{
    if (image.mWidth == 0 || image.mHeight == 0)
    {
        throw std::invalid_argument("Image dimensions are invalid");
    }

    if (image.mPixels.size() != image.mWidth * image.mHeight)
    {
        throw std::invalid_argument("Pixel data does not match image dimensions");
    }

    std::uint8_t* pixelData = new std::uint8_t[image.mPixels.size() * 4];
    for (unsigned i = 0; i < image.mPixels.size(); ++i)
    {
        const PNGColor& color = image.mPixels[i];
        pixelData[i * 4] = color.r;
        pixelData[i * 4 + 1] = color.g;
        pixelData[i * 4 + 2] = color.b;
        pixelData[i * 4 + 3] = color.a;
    }

    std::int32_t success = stbi_write_png(filename, image.mWidth, image.mHeight, 4, pixelData, image.mWidth * 4);

    delete[] pixelData;

    if (!success)
    {
        throw std::runtime_error("Failed to write PNG file");
    }
}
