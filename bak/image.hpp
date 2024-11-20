#pragma once

#include <cstdint>
#include <vector>

namespace BAK {

class FileBuffer;

class Image
{
public:
    Image(unsigned width, unsigned height, unsigned flags, bool isHighResLowCol);

    unsigned GetWidth() const;
    unsigned GetHeight() const;
    unsigned GetSize() const;
    bool IsHighResLowCol() const;
    uint8_t* GetPixels() const;
    const std::vector<uint8_t>& GetVector() const;
    uint8_t GetPixel(unsigned x, unsigned y) const;
    void Load(FileBuffer *buffer);
    void SetPixel(unsigned x, unsigned y, std::uint8_t color );

private:
    unsigned mWidth;
    unsigned mHeight;
    unsigned int mFlags;
    bool mIsHighResLowCol;
    std::vector<uint8_t> mPixels;
};

}
