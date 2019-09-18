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
#include "FontResource.h"
#include "MediaToolkit.h"

FontResource::FontResource()
        : TaggedResource()
        , font(0)
{}

FontResource::~FontResource()
{
    Clear();
}

Font*
FontResource::GetFont() const
{
    return font;
}

void
FontResource::Clear()
{
    delete font;
}

void
FontResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        Split(buffer);
        FileBuffer *fntbuf;
        if (!Find(TAG_FNT, fntbuf))
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        font = new Font;
        fntbuf->Skip(2);
        font->SetHeight((unsigned int)fntbuf->GetUint8());
        fntbuf->Skip(1);
        font->SetFirst((unsigned int)fntbuf->GetUint8());
        unsigned int numChars = (unsigned int)fntbuf->GetUint8();
        fntbuf->Skip(2);
        if (fntbuf->GetUint8() != 0x01)
        {
            ClearTags();
            throw CompressionError(__FILE__, __LINE__);
        }
        unsigned int size = (unsigned int)fntbuf->GetUint32LE();
        FileBuffer *glyphbuf = new FileBuffer(size);
        fntbuf->DecompressRLE(glyphbuf);
        unsigned int *glyphOffset = new unsigned int [numChars];
        for (unsigned int i = 0; i < numChars; i++)
        {
            glyphOffset[i] = glyphbuf->GetUint16LE();
        }
        unsigned int glyphDataStart = glyphbuf->GetBytesDone();
        for (unsigned int i = 0; i < numChars; i++)
        {
            FontGlyph glyph;
            glyphbuf->Seek(glyphDataStart + i);
            glyph.width = (unsigned int)glyphbuf->GetUint8();
            glyphbuf->Seek(glyphDataStart + numChars + glyphOffset[i]);
            for (int j = 0; j < font->GetHeight(); j++)
            {
                glyph.data[j] = (uint16_t)glyphbuf->GetUint8() << 8;
                if (glyph.width > 8)
                {
                    glyph.data[j] += (uint16_t)glyphbuf->GetUint8();
                }
            }
            font->AddGlyph(glyph);
        }
        delete[] glyphOffset;
        delete glyphbuf;
        ClearTags();
    }
    catch (Exception &e)
    {
        e.Print("FontResource::Load");
        ClearTags();
        throw;
    }
}

unsigned int
FontResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("FontResource::Save");
        throw;
    }
}
