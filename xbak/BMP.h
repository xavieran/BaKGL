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

#ifndef BMP_H
#define BMP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Image.h"
#include "Palette.h"

class BMP
{
private:
    Image *image;
    Palette *palette;
public:
    BMP();
    virtual ~BMP();
    Image * GetImage() const;
    void SetImage ( Image *img );
    Palette * GetPalette() const;
    void SetPalette ( Palette *pal );
    void Load ( const std::string &name );
    void Save ( const std::string &name );
};

#endif
