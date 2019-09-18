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

#include "Orientation.h"

Orientation::Orientation(const int head)
: heading(head)
{
}

Orientation::~Orientation()
{
}

int Orientation::GetHeading() const
{
    return heading.Get();
}

void Orientation::SetHeading(const int head)
{
    heading = Angle(head);
}

const Angle & Orientation::GetAngle() const
{
    return heading;
}

float Orientation::GetCos() const
{
    return heading.GetCos();
}

float Orientation::GetSin() const
{
    return heading.GetSin();
}

void Orientation::AdjustHeading(const int delta)
{
    heading += Angle(delta);
}
