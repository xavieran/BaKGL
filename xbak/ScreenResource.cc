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

#include "Exception.h"
#include "ScreenResource.h"

ScreenResource::ScreenResource()
        : image(0)
{
}

ScreenResource::~ScreenResource()
{
    Clear();
}

Image * ScreenResource::GetImage()
{
    return image;
}

void ScreenResource::Clear()
{
    if (image)
    {
        delete image;
        image = 0;
    }
}

void ScreenResource::Load(FileBuffer *buffer)
{
    try
    {
        bool isBookScreen = false;
        Clear();
        if (buffer->GetUint16LE() != 0x27b6)
        {
            buffer->Rewind();
            isBookScreen = true;
        }
        if (buffer->GetUint8() != 0x02)
        {
            throw DataCorruption(__FILE__, __LINE__);
        }
        FileBuffer *decompressed = new FileBuffer(buffer->GetUint32LE());
        buffer->DecompressLZW(decompressed);
        image = isBookScreen ? new Image(BOOK_SCREEN_WIDTH, BOOK_SCREEN_HEIGHT, true) : new Image(SCREEN_WIDTH, SCREEN_HEIGHT);
        image->Load(decompressed);
        delete decompressed;
    }
    catch (Exception &e)
    {
        e.Print("ScreenResource::Load");
        throw;
    }
}

unsigned int ScreenResource::Save(FileBuffer *buffer)
{
    try
    {
        //buffer->PutUint16LE(0x27b6);
        buffer->PutUint8(0x02);
        FileBuffer *decompressed = new FileBuffer(SCREEN_WIDTH * SCREEN_HEIGHT);
        unsigned int size = image->Save(decompressed);
        buffer->PutUint32LE(size);
        FileBuffer *compressed = new FileBuffer(size);
        decompressed->Rewind();
        size = decompressed->CompressLZW(compressed);
        compressed->Rewind();
        buffer->CopyFrom(compressed, size);
        delete compressed;
        delete decompressed;
        return size;
    }
    catch (Exception &e)
    {
        e.Print("ScreenResource::Save");
        throw;
    }
}
