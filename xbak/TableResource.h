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

#ifndef TABLE_RESOURCE_H
#define TABLE_RESOURCE_H

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Geometry.h"
#include "TaggedResource.h"

/* Entity Flags */
const unsigned int EF_TERRAIN   = 0x00;
const unsigned int EF_UNBOUNDED = 0x20;
const unsigned int EF_2D_OBJECT = 0x40;

/* Entity Type */
const unsigned int ET_TERRAIN    =  0;
const unsigned int ET_EXTERIOR   =  1;
const unsigned int ET_BRIDGE     =  2;
const unsigned int ET_INTERIOR   =  3;
const unsigned int ET_HILL       =  4;
const unsigned int ET_TREE       =  5;
const unsigned int ET_CHEST      =  6;
const unsigned int ET_DEADBODY1  =  7;
const unsigned int ET_FENCE      =  8;
const unsigned int ET_GATE       =  9;
const unsigned int ET_BUILDING   = 10;
const unsigned int ET_TOMBSTONE  = 12;
const unsigned int ET_SIGN       = 13;
const unsigned int ET_ROOM       = 14;
const unsigned int ET_PIT        = 15;
const unsigned int ET_DEADBODY2  = 16;
const unsigned int ET_DIRTPILE   = 17;
const unsigned int ET_CORN       = 18;
const unsigned int ET_FIRE       = 19;
const unsigned int ET_ENTRANCE   = 20;
const unsigned int ET_GROVE      = 21;
const unsigned int ET_FERN       = 22;
const unsigned int ET_DOOR       = 23;
const unsigned int ET_CRYST      = 24;
const unsigned int ET_ROCKPILE   = 25;
const unsigned int ET_BUSH1      = 26;
const unsigned int ET_BUSH2      = 27;
const unsigned int ET_BUSH3      = 28;
const unsigned int ET_SLAB       = 29;
const unsigned int ET_STUMP      = 30;
const unsigned int ET_WELL       = 31;
const unsigned int ET_ENGINE     = 33;
const unsigned int ET_SCARECROW  = 34;
const unsigned int ET_TRAP       = 35;
const unsigned int ET_CATAPULT   = 36;
const unsigned int ET_COLUMN     = 37;
const unsigned int ET_LANDSCAPE  = 38;
const unsigned int ET_MOUNTAIN   = 39;
const unsigned int ET_BAG        = 41;
const unsigned int ET_LADDER     = 42;

/* Terrain Type */
const unsigned int TT_NULL      = 0;
const unsigned int TT_INTERIOR  = 6;
const unsigned int TT_EXTERIOR  = 7;
const unsigned int TT_LANDSCAPE = 8;

/* Terrain Class */
const unsigned int TC_FIELD     = 0;
const unsigned int TC_LANDSCAPE = 1;
const unsigned int TC_OTHER     = 2;

class GidInfo
{
    public:
        unsigned int xradius;
        unsigned int yradius;
        unsigned int flags;
        std::vector <Vector2D *> textureCoords;
        std::vector <Vector2D *> otherCoords;
        GidInfo();
        ~GidInfo();
};

class DatInfo
{
    public:
        unsigned int entityFlags;
        unsigned int entityType;
        unsigned int terrainType;
        unsigned int terrainClass;
        unsigned int sprite;
        Vector3D min;
        Vector3D max;
        Vector3D pos;
        std::vector<Vector3D *> vertices;
        DatInfo();
        ~DatInfo();
};

class TableResource
    : public TaggedResource
{
    private:
        std::vector<std::string> mapItems;
        std::vector<DatInfo *> datItems;
        std::vector<GidInfo *> gidItems;
    public:
        TableResource();
        virtual ~TableResource();
        unsigned int GetMapSize() const;
        std::string& GetMapItem ( const unsigned int i );
        unsigned int GetDatSize() const;
        DatInfo* GetDatItem ( const unsigned int i );
        unsigned int GetGidSize() const;
        GidInfo* GetGidItem ( const unsigned int i );
        void Clear();
        void Load ( FileBuffer *buffer );
        unsigned int Save ( FileBuffer *buffer );
};

#endif
