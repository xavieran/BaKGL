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

#include "Exception.h"
#include "TableResource.h"

DatInfo::DatInfo()
{
}

DatInfo::~DatInfo()
{
    for (std::vector<Vector3D*>::iterator it = vertices.begin(); it != vertices.end(); ++it)
    {
        delete (*it);
    }
    vertices.clear();
}

GidInfo::GidInfo()
{
}

GidInfo::~GidInfo()
{
    for (std::vector<Vector2D*>::iterator it = textureCoords.begin(); it != textureCoords.end(); ++it)
    {
        delete (*it);
    }
    textureCoords.clear();
    for (std::vector<Vector2D*>::iterator it = otherCoords.begin(); it != otherCoords.end(); ++it)
    {
        delete (*it);
    }
    otherCoords.clear();
}

TableResource::TableResource()
        : TaggedResource()
        , mapItems()
{
}

TableResource::~TableResource()
{
    Clear();
}

unsigned int
TableResource::GetMapSize() const
{
    return mapItems.size();
}

std::string&
TableResource::GetMapItem(const unsigned int i)
{
    return mapItems[i];
}

unsigned int
TableResource::GetDatSize() const
{
    return datItems.size();
}

DatInfo*
TableResource::GetDatItem(const unsigned int i)
{
    return datItems[i];
}

unsigned int
TableResource::GetGidSize() const
{
    return gidItems.size();
}

GidInfo*
TableResource::GetGidItem(const unsigned int i)
{
    return gidItems[i];
}

void
TableResource::Clear()
{
    mapItems.clear();
    for (std::vector<DatInfo *>::iterator it = datItems.begin(); it != datItems.end(); ++it)
    {
        delete (*it);
    }
    datItems.clear();
    for (std::vector<GidInfo *>::iterator it = gidItems.begin(); it != gidItems.end(); ++it)
    {
        delete (*it);
    }
    gidItems.clear();
}

void
TableResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        Split(buffer);
        FileBuffer *mapbuf;
        FileBuffer *appbuf;
        FileBuffer *gidbuf;
        FileBuffer *datbuf;
        if (!Find(TAG_MAP, mapbuf) ||
                !Find(TAG_APP, appbuf) ||
                !Find(TAG_GID, gidbuf) ||
                !Find(TAG_DAT, datbuf))
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        mapbuf->Skip(2);
        unsigned int numMapItems = mapbuf->GetUint16LE();
        unsigned int *mapOffset = new unsigned int [numMapItems];
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            mapOffset[i] = mapbuf->GetUint16LE();
        }
        mapbuf->Skip(2);
        unsigned int mapDataStart = mapbuf->GetBytesDone();
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            mapbuf->Seek(mapDataStart + mapOffset[i]);
            std::string item = mapbuf->GetString();
            mapItems.push_back(item);
        }
        delete[] mapOffset;

        unsigned int numAppItems = appbuf->GetUint16LE();
        unsigned int appDataSize = appbuf->GetUint16LE();
        for (unsigned int i = 0; i< numAppItems; i++)
        {
            appbuf->Skip(appDataSize);
        }

        unsigned int *gidOffset = new unsigned int [numMapItems];
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            gidOffset[i] = (gidbuf->GetUint16LE() & 0x000f) + (gidbuf->GetUint16LE() << 4);
        }
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            gidbuf->Seek(gidOffset[i]);
            GidInfo *item = new GidInfo();
            item->xradius = gidbuf->GetUint16LE();
            item->yradius = gidbuf->GetUint16LE();
            bool more = gidbuf->GetUint16LE() > 0;
            item->flags = gidbuf->GetUint16LE();
            if (more)
            {
                gidbuf->Skip(2);
                unsigned int n = gidbuf->GetUint16LE();
                gidbuf->Skip(2);
                for (unsigned int j = 0; j < n; j++)
                {
                    int u = gidbuf->GetSint8();
                    int v = gidbuf->GetSint8();
                    int x = gidbuf->GetSint16LE();
                    int y = gidbuf->GetSint16LE();
                    item->textureCoords.push_back(new Vector2D(u, v));
                    item->otherCoords.push_back(new Vector2D(x, y));
                }
            }
            gidItems.push_back(item);
        }
        delete[] gidOffset;

        unsigned int *datOffset = new unsigned int [numMapItems];
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            datOffset[i] = (datbuf->GetUint16LE() & 0x000f) + (datbuf->GetUint16LE() << 4);
        }
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            datbuf->Seek(datOffset[i]);
            DatInfo *item = new DatInfo();
            item->entityFlags = datbuf->GetUint8();
            item->entityType = datbuf->GetUint8();
            item->terrainType = datbuf->GetUint8();
            item->terrainClass = datbuf->GetUint8();
            datbuf->Skip(4);
            bool more = datbuf->GetUint16LE() > 0;
            datbuf->Skip(4);
            if (more)
            {
                if (!(item->entityFlags & EF_UNBOUNDED))
                {
                    item->min.SetX(datbuf->GetSint16LE());
                    item->min.SetY(datbuf->GetSint16LE());
                    item->min.SetZ(datbuf->GetSint16LE());
                    item->max.SetX(datbuf->GetSint16LE());
                    item->max.SetY(datbuf->GetSint16LE());
                    item->max.SetZ(datbuf->GetSint16LE());
                }
                datbuf->Skip(2);
                unsigned int n = datbuf->GetUint16LE();
                datbuf->Skip(2);
                for (unsigned int j = 0; j < n; j++)
                {
                    datbuf->Skip(14);
                }
                if (item->terrainType != TT_NULL)
                {
                    if (item->terrainClass == TC_FIELD)
                    {
                        item->pos.SetX(datbuf->GetSint16LE());
                        item->pos.SetY(datbuf->GetSint16LE());
                        item->pos.SetZ(datbuf->GetSint16LE());
                    }
                    datbuf->Skip(6);
                    int x = datbuf->GetSint16LE();
                    int y = datbuf->GetSint16LE();
                    int z = datbuf->GetSint16LE();
                    while ((item->min.GetX() <= x) && (x <= item->max.GetX()) &&
                           (item->min.GetY() <= y) && (y <= item->max.GetY()) &&
                           (item->min.GetZ() <= z) && (z <= item->max.GetZ()))
                    {
                        item->vertices.push_back(new Vector3D(x, y, z));
                        x = datbuf->GetSint16LE();
                        y = datbuf->GetSint16LE();
                        z = datbuf->GetSint16LE();
                    }
                }
                if ((item->entityFlags & EF_UNBOUNDED) && (item->entityFlags & EF_2D_OBJECT) && (n == 1))
                {
                    datbuf->Skip(2);
                    item->sprite = datbuf->GetUint16LE();
                    datbuf->Skip(4);
                }
                else
                {
                    item->sprite = (unsigned int) -1;
                }
            }
            datItems.push_back(item);
        }
        delete[] datOffset;

        ClearTags();
    }
    catch (Exception &e)
    {
        e.Print("TableResource::Load");
        ClearTags();
        throw;
    }
}

unsigned int
TableResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("TableResource::Save");
        throw;
    }
}
