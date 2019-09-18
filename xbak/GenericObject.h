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

#ifndef GENERIC_OBJECT_H
#define GENERIC_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Camera.h"
#include "Vertex.h"

class GenericObject
{
    protected:
        Vertex pos;
    public:
        GenericObject(const Vector2D& p);
        virtual ~GenericObject();
        int GetAngle();
        unsigned int GetDistance();
        virtual void CalculateRelativePosition ( const Vector2D &p ) = 0;
        virtual bool IsInView ( const Angle & heading, unsigned int & distance ) = 0;
        virtual void DrawFirstPerson ( const int x, const int y, const int w, const int h, Camera *cam ) = 0;
        virtual void DrawTopDown() = 0;
};

#endif
