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

#include "MediaToolkit.h"
#include "PatternPolygonObject.h"

PatternPolygonObject::PatternPolygonObject(const Vector2D& p, const Image *image)
        : PolygonObject(p)
        , texture(image)
{
}

PatternPolygonObject::~PatternPolygonObject()
{
}

void PatternPolygonObject::DrawFirstPerson(const int x, const int y, const int w, const int h, Camera *cam)
{
    const int TERRAIN_YOFFSET = 81;
    int offset = (((cam->GetHeading() * 16) + ((cam->GetPos().GetX() + cam->GetPos().GetY()) / 100)) % (texture->GetWidth() / 3));
    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        Vector2D v = vertices[i].ToFirstPerson(w, h, cam->GetHeading());
        xCoords[i] = v.GetX();
        yCoords[i] = v.GetY();
    }
    MediaToolkit::GetInstance()->GetVideo()->FillPolygon(xCoords, yCoords, vertices.size(), texture->GetPixels(),
                                                         offset - x, TERRAIN_YOFFSET - y - h + TERRAIN_HEIGHT, texture->GetWidth());
}

void PatternPolygonObject::DrawTopDown()
{
}
