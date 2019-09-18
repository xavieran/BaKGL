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

#include <iostream>

#include "Exception.h"
#include "TaggedImageResource.h"

TaggedImageResource::TaggedImageResource()
        : numImages(0)
        , images()
{
}

TaggedImageResource::~TaggedImageResource()
{
    Clear();
}

unsigned int TaggedImageResource::GetNumImages() const
{
    return images.size();
}

Image * TaggedImageResource::GetImage ( unsigned int n ) const
{
    return images[n];
}

void TaggedImageResource::Clear()
{
    for ( std::vector<Image*>::iterator it = images.begin(); it != images.end(); ++it )
    {
        delete ( *it );
    }
    images.clear();
}

void TaggedImageResource::Load ( FileBuffer *buffer )
{
    try
    {
        Clear();
        Split(buffer);
        FileBuffer *infbuf;
        FileBuffer *binbuf;
        if (!Find(TAG_INF, infbuf) || !Find(TAG_BIN, binbuf))
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        if (binbuf->GetUint8() != 0x02)
        {
            throw DataCorruption(__FILE__, __LINE__);
        }
        FileBuffer *decompressed = new FileBuffer(binbuf->GetUint32LE());
        binbuf->DecompressLZW(decompressed);
        numImages = infbuf->GetUint16LE();
        int *width = new int[numImages];
        int *height = new int[numImages];
        for (unsigned int i = 0; i < numImages; i++)
        {
            width[i] = infbuf->GetSint16LE();
        }
        for (unsigned int i = 0; i < numImages; i++)
        {
            height[i] = infbuf->GetSint16LE();
        }
        for (unsigned int i = 0; i < numImages; i++)
        {
            Image *img = new Image(width[i], height[i], true);
            img->Load(decompressed);
            images.push_back(img);
        }
        delete[] width;
        delete[] height;
    }
    catch (Exception &e)
    {
        e.Print("TaggedImageResource::Load");
        throw;
    }
}

unsigned int TaggedImageResource::Save ( FileBuffer *buffer )
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("TaggedImageResource::Save");
        throw;
    }
}
