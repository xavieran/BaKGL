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

#ifndef CAMERA_H
#define CAMERA_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Orientation.h"
#include "Position.h"
#include "Subject.h"

class Camera
    : public Subject
{
    private:
        Position position;
        Orientation orientation;
    public:
        Camera ( const Vector2D &p, const int heading );
        ~Camera();
        const Position & GetPosition() const;
        const Vector2D & GetPos() const;
        void SetPosition ( const Vector2D &p );
        const Orientation & GetOrientation() const;
        const Angle & GetAngle() const;
        int GetHeading() const;
        void SetHeading ( const int heading );
        void Move ( const int delta );
        void Turn ( const int delta );
};

#endif
