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

#include "Camera.h"

Camera::Camera(const Vector2D &p, const int heading)
        : position(p)
        , orientation(heading)
{
}

Camera::~Camera()
{
}

const Position & Camera::GetPosition() const
{
    return position;
}

const Vector2D & Camera::GetPos() const
{
    return position.GetPos();
}

void Camera::SetPosition(const Vector2D &p)
{
    position.SetPos(p);
    Notify();
}

const Orientation & Camera::GetOrientation() const
{
    return orientation;
}

const Angle & Camera::GetAngle() const
{
    return orientation.GetAngle();
}

int Camera::GetHeading() const
{
    return orientation.GetHeading();
}

void Camera::SetHeading(const int heading)
{
    orientation.SetHeading(heading);
    Notify();
}

void Camera::Move(const int delta)
{
    position.Adjust((int)((float)delta * orientation.GetSin()),
                    (int)((float)delta * orientation.GetCos()));
    Notify();
}

void Camera::Turn(const int delta)
{
    orientation.AdjustHeading(delta);
    Notify();
}
