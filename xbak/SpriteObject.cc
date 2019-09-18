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

#include "Defines.h"
#include "SpriteObject.h"

SpriteObject::SpriteObject(const Vector2D &p, Image *img)
        : GenericObject(p)
        , sprite(img)
{
}

SpriteObject::~SpriteObject()
{
}

void SpriteObject::CalculateRelativePosition(const Vector2D &p)
{
    pos.CalculateRelativePosition(p);
}

bool SpriteObject::IsInView(const Angle & heading, unsigned int & distance)
{
    if (pos.IsInView(heading))
    {
        distance = pos.GetDistance();
        return true;
    }
    return false;
}

void SpriteObject::DrawFirstPerson(const int x, const int y, const int w, const int h, Camera *cam)
{
    Image *image = new Image((int)((float)sprite->GetWidth() * pos.GetDistanceFactor()), (int)((float)sprite->GetHeight() * pos.GetDistanceFactor()), sprite);
    Vector2D v = pos.ToFirstPerson(w, h, cam->GetOrientation().GetAngle());
    int ww = MIN(image->GetWidth(), w - v.GetX());
    int hh = image->GetHeight();
    image->Draw(x + v.GetX(), y + v.GetY() - hh, 0, 0, ww, hh, 0);
    delete image;
}

void SpriteObject::DrawTopDown()
{
}
