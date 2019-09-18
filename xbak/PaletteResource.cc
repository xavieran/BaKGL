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
#include "PaletteResource.h"

PaletteResource::PaletteResource()
        : TaggedResource()
{
    palette = new Palette(0);
}

PaletteResource::~PaletteResource()
{
    Clear();
}

Palette *
PaletteResource::GetPalette() const
{
    return palette;
}

void
PaletteResource::Clear()
{
    if (palette)
    {
        delete palette;
        palette = 0;
    }
}

void
PaletteResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        Split(buffer);
        FileBuffer *vgabuf;
        if (!Find(TAG_VGA, vgabuf))
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        unsigned int size = vgabuf->GetSize() / 3;
        palette = new Palette(size);
        for (unsigned int i = 0; i < size; i++)
        {
            Color c;
            c.r = (vgabuf->GetUint8() << 2);
            c.g = (vgabuf->GetUint8() << 2);
            c.b = (vgabuf->GetUint8() << 2);
            c.a = 0;
            palette->SetColor(i, c);
        }
        ClearTags();
    }
    catch (Exception &e)
    {
        e.Print("PaletteResource::Load");
        ClearTags();
        throw;
    }
}

unsigned int
PaletteResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("PaletteResource::Save");
        throw;
    }
}
