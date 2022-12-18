#include "bak/image.hpp"

#include "com/assert.hpp"

#include <cstring>

namespace BAK {

const unsigned int FLAG_XYSWAPPED  = 0x20;
const unsigned int FLAG_UNKNOWN    = 0x40;
const unsigned int FLAG_COMPRESSED = 0x80;

Image::Image(unsigned width, unsigned height, unsigned flags, bool isHighResLowCol)
:
    mWidth(width),
    mHeight(height),
    mFlags(flags),
    mIsHighResLowCol(isHighResLowCol),
    mPixels()
{
    mPixels.reserve(width * height);
}

unsigned Image::GetWidth() const
{
    return mWidth;
}

unsigned Image::GetHeight() const
{
    return mHeight;
}

unsigned Image::GetSize() const
{
    return mWidth * mHeight;
}

bool Image::IsHighResLowCol() const
{
    return mIsHighResLowCol;
}

uint8_t * Image::GetPixels() const
{
    // FIXME!
    return const_cast<uint8_t *>(mPixels.data());
}

void Image::SetPixel(unsigned x, unsigned y, std::uint8_t color)
{
    if ((x < mWidth) && (y < mHeight))
    {
        mPixels[x + mWidth * y] = color;
    }
}

void Image::Load(FileBuffer *buffer)
{
    ASSERT(mPixels.empty());

    FileBuffer *imgbuf;
    if (mFlags & FLAG_COMPRESSED)
    {
        imgbuf = new FileBuffer(mWidth * mHeight);
        buffer->DecompressRLE(imgbuf);
    }
    else
    {
        imgbuf = buffer;
    }

    if (mFlags & FLAG_XYSWAPPED)
    {
        for (unsigned x = 0; x < mWidth; x++)
        {
            for (unsigned y = 0; y < mHeight; y++)
            {
                SetPixel(x, y, imgbuf->GetUint8());
            }
        }
    }
    else
    {
        if (IsHighResLowCol())
        {
            for (unsigned y = 0; y < mHeight; y++)
            {
                for (unsigned x = 0; x < mWidth; x++)
                {
                    std::uint8_t c = imgbuf->GetUint8();
                    SetPixel(x, y, (c & 0xf0) >> 4);
                    x++;
                    SetPixel(x, y, c & 0x0f);
                }
            }
        }
        else
        {
            for (unsigned i = 0; i < GetSize(); i++)
            {
                mPixels.emplace_back(imgbuf->GetUint8());
            }
        }
    }

    if (mFlags & FLAG_COMPRESSED)
    {
        delete imgbuf;
    }
}

}
