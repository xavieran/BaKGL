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

#ifndef ZONE_H
#define ZONE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ImageResource.h"
#include "TableResource.h"
#include "TileWorldResource.h"

const unsigned int MAX_TILES = 32;

class Zone
{
private:
    ImageResource horizon;
    Image *terrain;
    Image *popUp;
    std::vector<Image *> sprites;
    std::map<const std::pair<unsigned int, unsigned int>, TileWorldResource *> tiles;
    TableResource *table;
public:
    Zone();
    ~Zone();
    void Clear();
    void Load ( const unsigned int n );
    Image* GetHorizon ( const unsigned int n );
    Image* GetTerrain() const;
    Image* GetPopUp() const;
    Image* GetSprite ( const unsigned int n );
    TileWorldResource* GetTile ( const unsigned int x, const unsigned int y );
    TableResource* GetTable();
};

#endif
