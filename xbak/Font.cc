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
#include "Font.h"
#include "MediaToolkit.h"

Font::Font()
        : first ( 0 )
        , height ( 0 )
{}

Font::~Font()
{
    fontGlyphs.clear();
}

unsigned int Font::GetFirst() const
{
    return first;
}

void Font::SetFirst ( unsigned int n )
{
    first = n;
}

int Font::GetHeight() const
{
    return height;
}

void Font::SetHeight ( int h )
{
    height = h;
}

int Font::GetWidth ( unsigned int n ) const
{
    if ( n < fontGlyphs.size() )
    {
        return fontGlyphs[n].width;
    }
    else
    {
        throw IndexOutOfRange ( __FILE__, __LINE__ );
    }
}

unsigned int Font::GetSize() const
{
    return fontGlyphs.size();
}

FontGlyph& Font::GetGlyph ( unsigned int n )
{
    if ( n < fontGlyphs.size() )
    {
        return fontGlyphs[n];
    }
    else
    {
        throw IndexOutOfRange ( __FILE__, __LINE__ );
    }
}

void Font::AddGlyph ( FontGlyph& glyph )
{
    fontGlyphs.push_back ( glyph );
}

void Font::DrawChar ( int x, int y, int ch, int color, bool italic )
{
    Video *video = MediaToolkit::GetInstance()->GetVideo();
    if ( ( int ) ( ch - first ) >= 0 )
    {
        if ( italic )
        {
            video->DrawGlyphItalic ( x, y, fontGlyphs[ch - first].width, height, color, fontGlyphs[ch - first].data );
        }
        else
        {
            video->DrawGlyph ( x, y, fontGlyphs[ch - first].width, height, color, fontGlyphs[ch - first].data );
        }
    }
}
