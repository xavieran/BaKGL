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

#ifndef VERTEX_H
#define VERTEX_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Geometry.h"

const unsigned int MAX_VIEW_DISTANCE = 32000;
const unsigned int MIN_VIEW_DISTANCE = 32;

const int ANGLE_OF_VIEW = 12;
const int ANGLE_VIEW_DISTANCE = MAX_VIEW_DISTANCE / 2;

const int TERRAIN_HEIGHT = 38;

class Vertex
{
    protected:
        Vector3D pos;
        Vector3D relpos;
        Angle angle;
        unsigned int distance;
        float distanceFactor;
    public:
        Vertex();
        Vertex ( const Vector3D &p );
        virtual ~Vertex();
        Vertex& operator= ( const Vertex &v );
        const Vector3D& GetPosition() const;
        const Vector3D& GetRelativePosition() const;
        int GetAngle() const;
        unsigned int GetDistance() const;
        float GetDistanceFactor() const;
        Vector2D ToFirstPerson ( int w, int h, const Angle & heading );
        Vector2D ToTopDown ( int w, int h );
        void CalculateRelativePosition ( const Vector2D &p );
        bool IsInView ( const Angle & heading );
};

#endif
