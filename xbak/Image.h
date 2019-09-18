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

#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FileBuffer.h"

class Image
{
private:
    int width;
    int height;
    unsigned int flags;
    bool highres_lowcol;
    uint8_t *pixel;
public:
    Image ( const int w, const int h );
    Image ( const int w, const int h, const unsigned int f );
    Image ( const int w, const int h, const bool hrlc );
    Image ( const int w, const int h, const uint8_t *p );
    Image ( Image *img );
    Image ( const int w, const int h, Image *img );
    Image ( const int w, const int h, std::vector<Image *> &img );
    ~Image();
    int GetWidth() const;
    int GetHeight() const;
    unsigned int GetSize() const;
    unsigned int GetFlags() const;
    void SetFlags ( const unsigned int f );
    bool IsHighResLowCol() const;
    uint8_t GetPixel ( const int x, const int y ) const;
    uint8_t * GetPixels() const;
    void SetPixel ( const int x, const int y, const uint8_t color );
    void SetPixels ( uint8_t *data, unsigned int size = 0 );
    void HorizontalFlip();
    void VerticalFlip();
    void Fill ( const uint8_t color );
    void Load ( FileBuffer *buffer );
    unsigned int Save ( FileBuffer *buffer );
    void Read ( const int x, const int y );
    void Draw ( const int x, const int y ) const;
    void Draw ( const int x, const int y, const uint8_t transparent ) const;
    void Draw ( const int x, const int y, const int xoff, const int yoff, const int w, const int h ) const;
    void Draw ( const int x, const int y, const int xoff, const int yoff, const int w, const int h, const uint8_t transparent ) const;
};

#endif
