#pragma once

#include <cstdint>
#include <vector>

struct PNGColor
{
    std::uint8_t r, g, b, a;
};

struct PNGImage
{
    std::uint32_t mWidth;
    std::uint32_t mHeight;
    std::vector<PNGColor> mPixels;
};

PNGImage LoadPNG(const char* filename);

void WritePNG(const char* filename, const PNGImage&);
