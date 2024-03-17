#include "bak/imageStore.hpp"
#include "bak/dataTags.hpp"

#include "com/logger.hpp"

#include <iostream>

namespace BAK {

std::vector<Image> LoadImagesNormal(FileBuffer& fb)
{
    std::vector<Image> images{};

    const unsigned compression = fb.GetUint16LE();
    const unsigned numImages = fb.GetUint16LE();

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

std::vector<Image> LoadImagesTagged(FileBuffer& fb)
{
    auto infBuf = fb.Find(DataTag::INF);

    std::vector<Image> images{};
    unsigned imageCount = infBuf.GetUint16LE();
    for (unsigned i = 0; i < imageCount; i++)
    {
        const auto width = infBuf.GetUint16LE();
        const auto start = infBuf.Tell();
        infBuf.Skip(2 * (imageCount - 1));
        const auto height = infBuf.GetUint16LE();
        infBuf.Seek(start);
        images.emplace_back(width, height, 0, true);
    }

    auto binBuf = fb.Find(DataTag::BIN);
    auto compression = binBuf.GetUint8();
    auto size = binBuf.GetUint32LE();
    FileBuffer decompressed = FileBuffer(size);
    auto decompressedBytes = binBuf.DecompressLZW(&decompressed);
    for (unsigned i = 0; i < imageCount; i++)
    {
        images[i].Load(&decompressed);
    }

    return images;
}

std::vector<Image> LoadImages(FileBuffer& fb)
{
    const auto imageType = fb.GetUint16LE();
    if (imageType == 0x1066)
    {
        return LoadImagesNormal(fb);
    }
    else if (imageType == 0x4d42)
    {
        return LoadImagesTagged(fb);
    }
    else
    {
        throw std::runtime_error("Couldn't load images");
        return std::vector<Image>{};
    }
}

}
