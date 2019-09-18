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

#ifndef FONT_H
#define FONT_H

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include "alt_stdint.h"
#endif

const unsigned int MAX_FONT_HEIGHT = 16;

typedef uint16_t GlyphData[MAX_FONT_HEIGHT];

struct FontGlyph
{
    unsigned int width;
    GlyphData data;
};

class Font
{
private:
    unsigned int first;
    int height;
    std::vector<FontGlyph> fontGlyphs;
public:
    Font();
    virtual ~Font();
    unsigned int GetFirst() const;
    void SetFirst ( unsigned int n );
    int GetHeight() const;
    void SetHeight ( int h );
    int GetWidth ( unsigned int n ) const;
    unsigned int GetSize() const;
    FontGlyph& GetGlyph ( unsigned int n );
    void AddGlyph ( FontGlyph& glyph );
    void DrawChar ( int x, int y, int ch, int color, bool italic );
};

#endif
