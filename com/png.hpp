#pragma once

#include <vector>
#include <cstdint> // for uint8_t

struct PNGColor {
    uint8_t r, g, b, a;
};

struct PNGImage {
    std::uint32_t mWidth;
    std::uint32_t mHeight;
    std::vector<PNGColor> mPixels;
};

PNGImage LoadPNG(const char* filename);
