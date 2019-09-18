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
#include "Position.h"
#include "TileWorldResource.h"

Position::Position(const Vector2D &p)
        : pos(p)
        , cell(p.GetX() / TILE_SIZE, p.GetY() / TILE_SIZE)
{}

Position::Position(const Vector2D &p, const Vector2D &c)
        : pos(p)
        , cell(c)
{}

Position::~Position()
{
}

const Vector2D & Position::GetCell() const
{
    return cell;
}

const Vector2D & Position::GetPos() const
{
    return pos;
}

void Position::SetPos(const Vector2D &p)
{
    pos = p;
    cell = pos / (int)TILE_SIZE;
}

void Position::Adjust(const Vector2D &delta)
{
    pos += delta;
    cell = pos / (int)TILE_SIZE;
}

void Position::Adjust(const int deltaX, const int deltaY)
{
    pos += Vector2D(deltaX, deltaY);
    cell = pos / (int)TILE_SIZE;
}
