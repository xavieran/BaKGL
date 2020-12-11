#pragma once

#include "Image.h"
#include "Palette.h"

#include <iostream>
#include <cassert>


#pragma pack(push, 1)
struct BMP_Header
{
    char header[2] = {'B', 'M'};
    std::uint32_t bfSize;
    std::uint16_t bfReserved1 = 0;
    std::uint16_t bfReserved2 = 0;
    std::uint32_t bfOffsetBits;
};
#pragma pack(pop)
static_assert(sizeof(BMP_Header) == 14);

#pragma pack(push, 1)
struct BITMAPINFOHEADER
{
    std::uint32_t biSize = 40;
    std::uint32_t biWidth;
    std::uint32_t biHeight;
    std::uint16_t biPlanes = 1;
    std::uint16_t biBitCount = 8;
    std::uint32_t biCompression = 0;
    std::uint32_t biSizeImage = 0;
    std::uint32_t biXPelsPerMeter = 0;
    std::uint32_t biYPelsPerMeter = 0;
    std::uint32_t biClrUsed = 0;
    std::uint32_t biClrImportant= 0;
};
#pragma pack(pop)
static_assert(sizeof(BITMAPINFOHEADER) == 40);

#pragma pack(push, 1)
struct RGBQUAD
{
    std::uint8_t blue = 0;
    std::uint8_t green = 0;
    std::uint8_t red = 0;
    std::uint8_t reserved = 0;
};
#pragma pack(pop)
static_assert(sizeof(RGBQUAD) == 4);


void WriteBMP(
    std::ostream& out,
    const Image& image,
    const Palette& palette)
{
    BMP_Header header{};
    BITMAPINFOHEADER bitInfo{};
    bitInfo.biWidth = image.GetWidth();
    bitInfo.biHeight = image.GetHeight();

    header.bfOffsetBits
        = sizeof(BMP_Header)
        + sizeof(BITMAPINFOHEADER)
        + sizeof(RGBQUAD) * 256;

    header.bfSize
        = header.bfOffsetBits 
        + (bitInfo.biWidth * bitInfo.biHeight);

    out.write(reinterpret_cast<char*>(&header), sizeof(header));
    out.write(reinterpret_cast<char*>(&bitInfo), sizeof(bitInfo));

    for (unsigned int i = 0; i < 256; i++)
    {
        RGBQUAD color{};
        if (i < palette.GetSize())
        {
            color.blue = palette.GetColor(i).b;
            color.red  = palette.GetColor(i).r;
            color.green = palette.GetColor(i).g;
        }

        out.write(reinterpret_cast<char*>(&color), sizeof(color));
    }

    for (int y = (image.GetHeight() - 1); y >= 0; --y)
    {
        for (int x = 0; x < image.GetWidth(); x++)
        {
            char pixel = image.GetPixel(x,y);
            out.write(&pixel, 1);
        }
    }
}

