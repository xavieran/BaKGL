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

#ifndef POSITION_H
#define POSITION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Geometry.h"

const int MOVE_SIZE     = 400;
const int MOVE_FORWARD  = +1 * MOVE_SIZE;
const int MOVE_BACKWARD = -1 * MOVE_SIZE;

class Position
{
private:
    Vector2D pos;
    Vector2D cell;
public:
    Position ( const Vector2D &p );
    Position ( const Vector2D &p, const Vector2D &c );
    ~Position();
    const Vector2D & GetCell() const;
    const Vector2D & GetPos() const;
    void SetPos ( const Vector2D &p );
    void Adjust ( const Vector2D &delta );
    void Adjust ( const int deltaX, const int deltaY );
};

#endif
