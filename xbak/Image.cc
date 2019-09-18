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

#include <cstring>

#include "Exception.h"
#include "Image.h"
#include "MediaToolkit.h"

const unsigned int FLAG_XYSWAPPED  = 0x20;
const unsigned int FLAG_UNKNOWN    = 0x40;
const unsigned int FLAG_COMPRESSED = 0x80;

Image::Image(const int w, const int h)
        : width(w)
        , height(h)
        , flags(0)
        , highres_lowcol(false)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        memset(pixel, 0, width * height);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::Image(const int w, const int h, const unsigned int f)
        : width(w)
        , height(h)
        , flags(f)
        , highres_lowcol(false)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        memset(pixel, 0, width * height);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::Image(const int w, const int h, const bool hrlc)
        : width(w)
        , height(h)
        , flags(0)
        , highres_lowcol(hrlc)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        memset(pixel, 0, width * height);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::Image(const int w, const int h, const uint8_t *p)
        : width(w)
        , height(h)
        , flags(0)
        , highres_lowcol(false)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        memcpy(pixel, p, width * height);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::Image(Image *img)
        : width(img->width)
        , height(img->height)
        , flags(img->flags)
        , highres_lowcol(img->highres_lowcol)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        memcpy(pixel, img->pixel, width * height);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::Image(const int w, const int h, Image *img)
        : width(w)
        , height(h)
        , flags(img->flags)
        , highres_lowcol(img->highres_lowcol)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        uint8_t *p = pixel;
        float fx = (float)img->width / (float)width;
        float fy = (float)img->height / (float)height;
        for (int y = 0; y < height; y++)
        {
            uint8_t *prow = img->pixel + (int)(y * fy) * img->width;
            for (int x = 0; x < width; x++)
            {
                *p++ = *(prow + (int)(x * fx));
            }
        }
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::Image(const int w, const int h, std::vector<Image *> &img)
        : width(w)
        , height(h)
        , flags(img[0]->flags)
        , highres_lowcol(img[0]->highres_lowcol)
        , pixel(0)
{
    if ((width > 0) && (height > 0))
    {
        pixel = new uint8_t[width * height];
        uint8_t *p = pixel;
        if (width == img[0]->width)
        {
            unsigned int i = 0;
            int y = 0;
            do
            {
                uint8_t *q = img[i]->pixel + (y - i * width) * img[i]->width;
                memcpy(p, q, width);
                p += width;
                y++;
                if (y == height * ((int)i + 1))
                {
                    i++;
                    if (i == img.size())
                    {
                        i = 0;
                    }
                }
            }
            while (y < height);
        }
        if (height == img[0]->height)
        {
            for (int y = 0; y < height; y++)
            {
                unsigned int i = 0;
                int x = 0;
                do
                {
                    uint8_t *q = img[i]->pixel + y * img[i]->width;
                    memcpy(p, q, img[i]->width);
                    p += img[i]->width;
                    x += img[i]->width;
                    i++;
                    if (i == img.size())
                    {
                        i = 0;
                    }
                }
                while (x < width);
            }
        }
    }
    else
    {
        width = 0;
        height = 0;
    }
}

Image::~Image()
{
    if (pixel)
    {
        delete [] pixel;
    }
}

int Image::GetWidth() const
{
    return width;
}

int Image::GetHeight() const
{
    return height;
}

unsigned int Image::GetSize() const
{
    return (unsigned int)width * height;
}

unsigned int Image::GetFlags() const
{
    return flags;
}

void Image::SetFlags(const unsigned int f)
{
    flags = f;
}

bool Image::IsHighResLowCol() const
{
    return highres_lowcol;
}

uint8_t Image::GetPixel(const int x, const int y) const
{
    if ((pixel) && (x >= 0) && (x < width) && (y >= 0) && (y < height))
    {
        return pixel[x + width * y];
    }
    return 0;
}

uint8_t * Image::GetPixels() const
{
    return pixel;
}

void Image::SetPixel(const int x, const int y, const uint8_t color)
{
    if ((pixel) && (x >= 0) && (x < width) && (y >= 0) && (y < height))
    {
        pixel[x + width * y] = color;
    }
}

void Image::SetPixels(uint8_t *data, unsigned int size)
{
    if ((pixel) && (data))
    {
        if (size == 0)
        {
            size = (unsigned int)width * height;
        }
        memcpy(pixel, data, size);
    }
}

void Image::Fill(const uint8_t color)
{
    if (pixel)
    {
        memset(pixel, color, width * height);
    }
}

void Image::HorizontalFlip()
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width / 2; x++)
        {
            uint8_t h = pixel[x + width * y];
            pixel[x + width * y] = pixel[width - x - 1 + width * y];
            pixel[width - x - 1 + width * y] = h;
        }
    }
}

void Image::VerticalFlip()
{
    uint8_t *row = new uint8_t[width];
    for (int y = 0; y < height / 2; y++)
    {
        memcpy(row, pixel + width * y, width);
        memcpy(pixel + width * y, pixel + width * (height - y - 1), width);
        memcpy(pixel + width * (height - y - 1), row, width);
    }
    delete[] row;
}

void Image::Load(FileBuffer *buffer)
{
    try
    {
        if (pixel)
        {
            FileBuffer *imgbuf;
            if (flags & FLAG_COMPRESSED)
            {
                imgbuf = new FileBuffer(width * height);
                buffer->DecompressRLE(imgbuf);
            }
            else
            {
                imgbuf = buffer;
            }
            if (flags & FLAG_XYSWAPPED)
            {
                for (int x = 0; x < width; x++)
                {
                    for (int y = 0; y < height; y++)
                    {
                        SetPixel(x, y, imgbuf->GetUint8());
                    }
                }
            }
            else
            {
                if (highres_lowcol)
                {
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            uint8_t c = imgbuf->GetUint8();
                            SetPixel(x, y, (c & 0xf0) >> 4);
                            x++;
                            SetPixel(x, y, c & 0x0f);
                        }
                    }
                }
                else
                {
                    imgbuf->GetData(pixel, width * height);
                }
            }
            if (flags & FLAG_COMPRESSED)
            {
                delete imgbuf;
            }
        }
    }
    catch (Exception &e)
    {
        e.Print("Image::Load");
        throw;
    }
}

unsigned int Image::Save(FileBuffer *buffer)
{
    try
    {
        if (pixel)
        {
            FileBuffer *imgbuf = new FileBuffer(width * height);
            if (flags & FLAG_XYSWAPPED)
            {
                for (int x = 0; x < width; x++)
                {
                    for (int y = 0; y < height; y++)
                    {
                        imgbuf->PutUint8(GetPixel(x, y));
                    }
                }
            }
            else
            {
                if (highres_lowcol)
                {
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            uint8_t c1 = GetPixel(x, y);
                            x++;
                            uint8_t c2 = GetPixel(x, y);
                            imgbuf->PutUint8(((c1 & 0x0f) << 4) | (c2 & 0x0f));
                        }
                    }
                }
                else
                {
                    imgbuf->PutData(pixel, width * height);
                }
            }
            imgbuf->Rewind();
            unsigned int size;
            if (flags & FLAG_COMPRESSED)
            {
                FileBuffer *compressed = new FileBuffer(width * height);
                size = imgbuf->CompressRLE(compressed);
                buffer->CopyFrom(compressed, size);
                delete compressed;
            }
            else
            {
                size = width * height;
                buffer->CopyFrom(imgbuf, size);
            }
            delete imgbuf;
            return size;
        }
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("Image::Save");
        throw;
    }
    return 0;
}

void Image::Read(const int x, const int y)
{
    MediaToolkit::GetInstance()->GetVideo()->ReadImage(x, y, width, height, pixel);
}

void Image::Draw(const int x, const int y) const
{
    MediaToolkit::GetInstance()->GetVideo()->DrawImage(x, y, width, height, pixel);
}

void Image::Draw(const int x, const int y, const uint8_t transparent) const
{
    MediaToolkit::GetInstance()->GetVideo()->DrawImage(x, y, width, height, pixel, transparent);
}

void Image::Draw(const int x, const int y, const int xoff, const int yoff, const int w, const int h) const
{
    MediaToolkit::GetInstance()->GetVideo()->DrawImage(x, y, width, height, xoff, yoff, w, h, pixel);
}

void Image::Draw(const int x, const int y, const int xoff, const int yoff, const int w, const int h, const uint8_t transparent) const
{
    MediaToolkit::GetInstance()->GetVideo()->DrawImage(x, y, width, height, xoff, yoff, w, h, pixel, transparent);
}
