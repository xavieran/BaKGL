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

#include <cstdlib>

#include "Orientation.h"
#include "Vertex.h"

Vertex::Vertex()
        : pos(0, 0, 0)
        , relpos(0, 0, 0)
        , angle(0)
        , distance(0)
        , distanceFactor(1.0)
{
}

Vertex::Vertex(const Vector3D &p)
        : pos(p)
        , relpos(0, 0, 0)
        , angle(0)
        , distance(0)
        , distanceFactor(1.0)
{
}

Vertex::~Vertex()
{
}

Vertex& Vertex::operator= ( const Vertex &v )
{
    pos = v.pos;
    relpos = v.relpos;
    angle = v.angle;
    distance = v.distance;
    distanceFactor = v.distanceFactor;
    return *this;
}

const Vector3D& Vertex::GetPosition() const
{
    return pos;
}

const Vector3D& Vertex::GetRelativePosition() const
{
    return relpos;
}

int Vertex::GetAngle() const
{
    return angle.Get();
}

unsigned int Vertex::GetDistance() const
{
    return distance;
}

float Vertex::GetDistanceFactor() const
{
    return distanceFactor;
}

Vector2D Vertex::ToFirstPerson(int w, int h, const Angle &heading)
{
    const int ANGLE_AOV = 2 * ANGLE_OF_VIEW + 1;
    int x = (int)((float)w * (float)(angle.Get() - heading.Get() + ANGLE_OF_VIEW - 1) / (float)ANGLE_AOV);
    int y = h - (int)((float)TERRAIN_HEIGHT * (1.0 - distanceFactor) + ((float)relpos.GetZ() * (0.05 + 0.45 * distanceFactor)));
    return Vector2D(x, y);
}

Vector2D Vertex::ToTopDown(int , int )
{
    // TODO
    return Vector2D(0, 0);
}

void Vertex::CalculateRelativePosition(const Vector2D &p)
{
    relpos = pos - p;
    angle = Angle((ANGLE_SIZE / 4) - relpos.GetTheta());
    distance = relpos.GetRho();
    distanceFactor = 2.0 * ((float)MAX_VIEW_DISTANCE / ((float)MAX_VIEW_DISTANCE + (float)distance)) - 1.0;
    //distanceFactor = 1.0 - ((float)MAX_VIEW_DISTANCE / ((float)MAX_VIEW_DISTANCE + (float)distance));
}

bool Vertex::IsInView(const Angle &heading)
{
    if (distance > MAX_VIEW_DISTANCE)
    {
        return false;
    }
    return (abs(angle.Get() - heading.Get()) <= ANGLE_OF_VIEW);
}
