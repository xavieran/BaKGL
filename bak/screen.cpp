#include "bak/screen.hpp"

#include "bak/image.hpp"
#include "bak/fileBufferFactory.hpp"

namespace BAK {

constexpr unsigned SCREEN_WIDTH       = 320;
constexpr unsigned SCREEN_HEIGHT      = 200;
constexpr unsigned BOOK_SCREEN_WIDTH  = 640;
constexpr unsigned BOOK_SCREEN_HEIGHT = 350;

Image LoadScreenResource(FileBuffer& fb)
{
    bool isBookScreen = false;

    if (fb.GetUint16LE() != 0x27b6)
    {
        fb.Rewind();
        isBookScreen = true;
    }

    if (fb.GetUint8() != 0x02)
    {
        throw std::runtime_error("Could not load screen resource");
    }

    FileBuffer *decompressed = new FileBuffer(fb.GetUint32LE());
    fb.DecompressLZW(decompressed);
    auto image = isBookScreen
        ? Image(BOOK_SCREEN_WIDTH, BOOK_SCREEN_HEIGHT, 0, true)
        : Image(SCREEN_WIDTH, SCREEN_HEIGHT, 0, false);
    image.Load(decompressed);
    delete decompressed;
    return image;
}

}
