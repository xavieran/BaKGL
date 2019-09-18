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

#ifndef ORIENTATION_H
#define ORIENTATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Geometry.h"

const int NORTH = 0;
const int EAST  = ANGLE_SIZE / 4;
const int SOUTH = ANGLE_SIZE / 2;
const int WEST  = 3 * ANGLE_SIZE / 4;

const int TURN_SIZE  = 2;
const int TURN_LEFT  = -1 * TURN_SIZE;
const int TURN_RIGHT = +1 * TURN_SIZE;

class Orientation
{
    private:
        Angle heading;
    public:
        Orientation ( const int head );
        ~Orientation();
        const Angle & GetAngle() const;
        int GetHeading() const;
        void SetHeading ( const int head );
        float GetCos() const;
        float GetSin() const;
        void AdjustHeading ( const int delta );
};

#endif
