#include "com/png.hpp"


#define STB_IMAGE_IMPLEMENTATION
#include "com/stb_image.h"

#include <stdexcept>
#include <string>

PNGImage LoadPNG(const char* filename) {
    PNGImage image;

    int x, y, n;
    unsigned char* data = stbi_load(filename, &x, &y, &n, 4); // 4 for RGBA
    if (!data) {
        throw std::runtime_error("Failed to load PNG: " + std::string{filename});
    }

    image.mWidth = x;
    image.mHeight = y;
    image.mPixels.resize(x * y);

    for (int i = 0; i < x * y; ++i) {
        image.mPixels[i].r = data[i * 4 + 0];
        image.mPixels[i].g = data[i * 4 + 1];
        image.mPixels[i].b = data[i * 4 + 2];
        image.mPixels[i].a = data[i * 4 + 3];
    }

    stbi_image_free(data);
    return image;
}
