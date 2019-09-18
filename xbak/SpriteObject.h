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

#ifndef SPRITE_OBJECT_H
#define SPRITE_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GenericObject.h"
#include "Image.h"

class SpriteObject
    : public GenericObject
{
    private:
        Image *sprite;
    public:
        SpriteObject ( const Vector2D &p, Image *img );
        ~SpriteObject();
        void CalculateRelativePosition ( const Vector2D &p );
        bool IsInView ( const Angle & heading, unsigned int & distance );
        void DrawFirstPerson ( const int x, const int y, const int w, const int h, Camera *cam );
        void DrawTopDown();
};

#endif
