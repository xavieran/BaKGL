/*
 * This file is part of xBaK.
 *
 * xBaK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xBaK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xBaK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Guido de Jong <guidoj@users.sf.net>
 */

#include "BMP.h"
#include "Exception.h"

typedef struct _BitmapFileHeader
{
    char magic[2];
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
}
BitmapFileHeader;

typedef struct _BitmapInfoHeader
{
    uint32_t infoSize;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t xPelsPerMeter;
    uint32_t yPelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
}
BitmapInfoHeader;

const unsigned int BITMAP_FILE_HEADER_SIZE = 14;
const unsigned int BITMAP_INFO_HEADER_SIZE = 40;

BMP::BMP()
        : image(0)
        , palette(0)
{
}

BMP::~BMP()
{
}

Image * BMP::GetImage() const
{
    return image;
}

void BMP::SetImage(Image *img)
{
    image = img;
}

Palette * BMP::GetPalette() const
{
    return palette;
}

void BMP::SetPalette(Palette *pal)
{
    palette = pal;
}

void BMP::Load(const std::string &name)
{
    std::ifstream ifs;
    ifs.open(name.c_str(), std::ios::in | std::ios::binary);
    if (ifs.fail())
    {
        throw OpenError(__FILE__, __LINE__, "(" + name + ")");
    }

    FileBuffer bmpFileHdrBuffer(BITMAP_FILE_HEADER_SIZE);
    bmpFileHdrBuffer.Load(ifs);
    BitmapFileHeader bmpFileHdr;
    bmpFileHdrBuffer.GetData(bmpFileHdr.magic, 2);
    bmpFileHdr.fileSize = bmpFileHdrBuffer.GetUint32LE();
    bmpFileHdr.reserved1 = bmpFileHdrBuffer.GetUint16LE();
    bmpFileHdr.reserved2 = bmpFileHdrBuffer.GetUint16LE();
    bmpFileHdr.offset = bmpFileHdrBuffer.GetUint32LE();
    if ((bmpFileHdr.magic[0] != 'B') || (bmpFileHdr.magic[1] != 'M'))
    {
        throw DataCorruption(__FILE__, __LINE__, "BMP magic");
    }

    FileBuffer bmpInfoHdrBuffer(BITMAP_INFO_HEADER_SIZE);
    bmpInfoHdrBuffer.Load(ifs);
    BitmapInfoHeader bmpInfoHdr;
    bmpInfoHdr.infoSize = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.width = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.height = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.planes = bmpInfoHdrBuffer.GetUint16LE();
    bmpInfoHdr.bitCount = bmpInfoHdrBuffer.GetUint16LE();
    bmpInfoHdr.compression = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.imageSize = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.xPelsPerMeter = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.yPelsPerMeter = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.colorsUsed = bmpInfoHdrBuffer.GetUint32LE();
    bmpInfoHdr.colorsImportant = bmpInfoHdrBuffer.GetUint32LE();
    if (bmpInfoHdr.infoSize != BITMAP_INFO_HEADER_SIZE)
    {
        throw DataCorruption(__FILE__, __LINE__, "BMP info header size");
    }
    if (bmpInfoHdr.planes != 1)
    {
        throw DataCorruption(__FILE__, __LINE__, "BMP planes");
    }
    if (bmpInfoHdr.compression != 0)
    {
        throw DataCorruption(__FILE__, __LINE__, "BMP compression");
    }

    if (!palette)
    {
        throw NullPointer(__FILE__, __LINE__, "palette");
    }
    FileBuffer paletteBuffer(bmpInfoHdr.colorsUsed * 4);
    paletteBuffer.Load(ifs);
    if (palette->GetSize() != bmpInfoHdr.colorsUsed)
    {
        throw DataCorruption(__FILE__, __LINE__, "palette size");
    }
    for (unsigned int i = 0; i < bmpInfoHdr.colorsUsed; i++)
    {
        Color c;
        c.b = paletteBuffer.GetUint8();
        c.g = paletteBuffer.GetUint8();
        c.r = paletteBuffer.GetUint8();
        c.a = paletteBuffer.GetUint8();
        palette->SetColor(i, c);
    }

    if (!image)
    {
        throw NullPointer(__FILE__, __LINE__, "image");
    }
    FileBuffer imageBuffer(bmpInfoHdr.imageSize);
    imageBuffer.Load(ifs);
    if ((image->GetWidth() != bmpInfoHdr.width) || (image->GetHeight() != bmpInfoHdr.height))
    {
        throw DataCorruption(__FILE__, __LINE__, "image size");
    }
    uint8_t *pixelLine = image->GetPixels() + bmpInfoHdr.imageSize;
    while (pixelLine > image->GetPixels())
    {
        pixelLine -= bmpInfoHdr.width;
        imageBuffer.GetData(pixelLine, bmpInfoHdr.width);
    }

    ifs.close();
}

void BMP::Save(const std::string &name)
{
    std::ofstream ofs;

    ofs.open(name.c_str(), std::ios::out | std::ios::binary);
    if (ofs.fail())
    {
        throw OpenError(__FILE__, __LINE__, "(" + name + ")");
    }

    BitmapFileHeader bmpFileHdr;
    bmpFileHdr.magic[0] = 'B';
    bmpFileHdr.magic[1] = 'M';
    bmpFileHdr.fileSize = BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + palette->GetSize() * 4 + image->GetSize();
    bmpFileHdr.reserved1 = 0;
    bmpFileHdr.reserved2 = 0;
    bmpFileHdr.offset = BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + palette->GetSize() * 4;
    FileBuffer bmpFileHdrBuffer(BITMAP_FILE_HEADER_SIZE);
    bmpFileHdrBuffer.PutData(bmpFileHdr.magic, 2);
    bmpFileHdrBuffer.PutUint32LE(bmpFileHdr.fileSize);
    bmpFileHdrBuffer.PutUint16LE(bmpFileHdr.reserved1);
    bmpFileHdrBuffer.PutUint16LE(bmpFileHdr.reserved2);
    bmpFileHdrBuffer.PutUint32LE(bmpFileHdr.offset);
    bmpFileHdrBuffer.Save(ofs);

    BitmapInfoHeader bmpInfoHdr;
    bmpInfoHdr.infoSize = BITMAP_INFO_HEADER_SIZE;
    bmpInfoHdr.width = image->GetWidth();
    bmpInfoHdr.height = image->GetHeight();
    bmpInfoHdr.planes = 1;
    bmpInfoHdr.bitCount = WINDOW_BPP;
    bmpInfoHdr.compression = 0;
    bmpInfoHdr.imageSize = image->GetSize();
    bmpInfoHdr.xPelsPerMeter = 0;
    bmpInfoHdr.yPelsPerMeter = 0;
    bmpInfoHdr.colorsUsed = palette->GetSize();
    bmpInfoHdr.colorsImportant = 0;
    FileBuffer bmpInfoHdrBuffer(BITMAP_INFO_HEADER_SIZE);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.infoSize);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.width);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.height);
    bmpInfoHdrBuffer.PutUint16LE(bmpInfoHdr.planes);
    bmpInfoHdrBuffer.PutUint16LE(bmpInfoHdr.bitCount);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.compression);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.imageSize);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.xPelsPerMeter);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.yPelsPerMeter);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.colorsUsed);
    bmpInfoHdrBuffer.PutUint32LE(bmpInfoHdr.colorsImportant);
    bmpInfoHdrBuffer.Save(ofs);

    FileBuffer paletteBuffer(palette->GetSize() * 4);
    for (unsigned int i = 0; i < palette->GetSize(); i++)
    {
        Color c = palette->GetColor(i);
        paletteBuffer.PutUint8(c.b);
        paletteBuffer.PutUint8(c.g);
        paletteBuffer.PutUint8(c.r);
        paletteBuffer.PutUint8(c.a);
    }
    paletteBuffer.Save(ofs);

    FileBuffer imageBuffer(image->GetSize());
    uint8_t *pixelLine = image->GetPixels() + image->GetSize();
    while (pixelLine > image->GetPixels())
    {
        pixelLine -= image->GetWidth();
        imageBuffer.PutData(pixelLine, image->GetWidth());
    }
    imageBuffer.Save(ofs);

    ofs.close();
}
