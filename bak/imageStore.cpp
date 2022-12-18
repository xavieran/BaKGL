#include "bak/imageStore.hpp"

#include <iostream>

namespace BAK {

std::vector<Image> LoadImages(FileBuffer& fb)
{
    std::vector<Image> images{};

    if (fb.GetUint16LE() != 0x1066)
    {
        throw std::runtime_error("Couldn't load images");
    }

    const unsigned compression = fb.GetUint16LE();
    const unsigned numImages = fb.GetUint16LE();
    std::cout << "NNum Images: " << numImages << std::endl;

    std::vector<unsigned> imageSizes{};
    fb.Skip(2);
    unsigned int size = fb.GetUint32LE();
    for (unsigned i = 0; i < numImages; i++)
    {
        imageSizes.emplace_back(fb.GetUint16LE());
        unsigned flags = fb.GetUint16LE();
        unsigned width = fb.GetUint16LE();
        unsigned height = fb.GetUint16LE();
        images.emplace_back(width, height, flags, false);
    }

    if (compression == 1)
    {
        // Not sure why this is needed or if *2 is the right number
        size *= 2;
    }

    FileBuffer decompressed = FileBuffer(size);
    fb.Decompress(&decompressed, compression);
    for (unsigned int i = 0; i < numImages; i++)
    {
        auto imageBuffer = FileBuffer(imageSizes[i]);
        imageBuffer.Fill(&decompressed);
        images[i].Load(&imageBuffer);
    }

    return images;
}

}
