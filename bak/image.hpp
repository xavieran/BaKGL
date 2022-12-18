#pragma once

#include <vector>

#include "xbak/FileBuffer.h"

namespace BAK {

class Image
{
public:
    Image(unsigned width, unsigned height, unsigned flags, bool isHighResLowCol);

    unsigned GetWidth() const;
    unsigned GetHeight() const;
    unsigned GetSize() const;
    bool IsHighResLowCol() const;
    uint8_t * GetPixels() const;
    void Load(FileBuffer *buffer);

private:
    void SetPixel(unsigned x, unsigned y, std::uint8_t color );

    unsigned mWidth;
    unsigned mHeight;
    unsigned int mFlags;
    bool mIsHighResLowCol;
    std::vector<uint8_t> mPixels;
};

}
