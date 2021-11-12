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

#define DEBUG

#include "Exception.h"
#include "TableResource.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

DatInfo::DatInfo()
:
    entityFlags{0},
    entityType{0},
    terrainType{0},
    terrainClass{0},
    sprite{0},
    min{0,0,0},
    max{0,0,0},
    pos{0,0,0},
    vertices{},
    faceColors{},
    paletteSources{},
    faces{}
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
        , datItems()
        , gidItems()
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
        FileBuffer* mapbuf{nullptr};
        FileBuffer* appbuf{nullptr};
        FileBuffer* gidbuf{nullptr};
        FileBuffer* datbuf{nullptr};
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
#ifdef DEBUG
        std::cout << " NumMapItems: "  << numMapItems << std::endl;
#endif
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
        unsigned int *gidLength = new unsigned int [numMapItems];

        std::set<unsigned int> uniqGidOffs;
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            unsigned int lower = gidbuf->GetUint16LE();
            unsigned int upper = gidbuf->GetUint16LE();
            unsigned int offset = (upper << 4) + (lower & 0x000f);
#ifdef DEBUG
            std::cout << "Load GIDOff: " << i << " name: " 
                << mapItems[i] << std::hex << " upper: " << upper
                << " lower: " << lower << " off: " << offset <<  " buf: " 
                << std::dec << gidbuf->Tell() << std::endl;
#endif
            gidOffset[i] = offset;
            uniqGidOffs.emplace(offset);
        }

        for (unsigned int i = 0; i < numMapItems; i++)
        {
            auto offset = gidOffset[i];
            auto it = std::find(uniqGidOffs.begin(), uniqGidOffs.end(), offset);
            it++;
            if (it != uniqGidOffs.end())
                gidLength[i] = *it - offset;
            else
                gidLength[i] = 0;
        }

        for (unsigned int i = 0; i < (numMapItems - 1); i++)
        {
            gidbuf->Seek(gidOffset[i]);
#ifdef DEBUG
            std::cout << "GidItem: " << mapItems[i] << " " << i << " length: "
                << " i: " << std::hex << i << " length: " << std::dec << gidLength[i] << std::endl;
            gidbuf->Dump(gidLength[i]);
#endif
        }
    
        for (unsigned int i = 0; i < numMapItems; i++)
        {
#ifdef DEBUG
            std::cout << " gidItem: " << mapItems[i] << " " << i << " current: " << gidbuf->Tell()
                << " off: " << gidOffset[i] << std::endl;
#endif
            // Seek a fixed distance from the start of the gidbuf
            gidbuf->Seek(gidOffset[i]); 
            GidInfo *item = new GidInfo();

            item->xradius = gidbuf->GetUint16LE();
            item->yradius = gidbuf->GetUint16LE();
#ifdef DEBUG
            std::cout << "HitRad: (" << item->xradius << "," << item->yradius << ")"
                << std::endl;
#endif
            bool more = gidbuf->GetUint16LE() > 0;
            item->flags = gidbuf->GetUint16LE();

            if (more)
            {
                gidbuf->Skip(2);
                //unsigned int n = gidbuf->GetUint16LE();
                unsigned int n = gidbuf->GetUint8();
                gidbuf->Skip(1);
#ifdef DEBUG
                std::cout << "n: " << n << std::endl;
#endif
                gidbuf->Skip(2);
                for (unsigned int j = 0; j < n; j++)
                {
                    int u = gidbuf->GetSint8();
                    int v = gidbuf->GetSint8();
                    int x = gidbuf->GetSint16LE();
                    int y = gidbuf->GetSint16LE();
#ifdef DEBUG
                    std::cout << std::hex << "uvxyH " << u << " " << v << " " << x << " " << y << std::dec;
                    std::cout << " uvxy " << u << " " << v << " " << x << " " << y << std::endl; 
#endif
                    item->textureCoords.push_back(new Vector2D(u, v));
                    item->otherCoords.push_back(new Vector2D(x, y));
                }
            }
            gidItems.push_back(item);
        }
        delete[] gidOffset;
        delete[] gidLength;

        unsigned int *datOffset = new unsigned int [numMapItems];
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            unsigned int lower = datbuf->GetUint16LE();
            unsigned int upper = datbuf->GetUint16LE();
            unsigned int offset = (upper << 4) + (lower & 0x000f);
#ifdef DEBUG
            std::cout << "Load DatOff: " << i << " name: " 
                << mapItems[i] << std::hex << " upper: " << upper
                << " lower: " << lower << " off: " << offset <<  " buf: " 
                << std::dec << datbuf->Tell() << std::endl;
#endif
            datOffset[i] = offset;
        }

        for (unsigned int i = 0; i < (numMapItems - 1); i++)
        {
            datbuf->Seek(datOffset[i]);
            unsigned int length = datOffset[i + 1] - datOffset[i];
#ifdef DEBUG
            std::cout << "DatItem: " << mapItems[i] << " " << i << " length: " << length
                << " i: " << std::hex << i << " length: " << length << std::dec << std::endl;
            datbuf->Dump(length);
#endif
        }
        unsigned ii = numMapItems - 1;
        unsigned ilength = datbuf->GetBytesLeft();
#ifdef DEBUG
        std::cout << "DatItem: " << mapItems[ii] << " " << ii << " length: " << ilength
            << " i: " << std::hex << ii << " length: " << ilength << std::dec << std::endl;
#endif
        datbuf->Seek(datOffset[ii]);
#ifdef DEBUG
        datbuf->Dump(ilength);
#endif

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
#ifdef DEBUG
            std::cout << "\tDatItem: " << mapItems[i]  << " 0x" << std::hex << i << " f: " << item->entityFlags
                << " et: " << item->entityType << " tt: " << item->terrainType
                << " tc: " << item->terrainClass << " more: " << more 
                << std::dec << " - "  << datbuf->Tell() << std::endl;
#endif
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

                unsigned nComponents = datbuf->GetUint16LE();
#ifdef DEBUG
                std::cout << "Components: " << nComponents << std::endl;
#endif
                datbuf->Skip(2); // Seems important...
                unsigned nVertices = 0;
                unsigned prevV = 0;
                // Which component do we go over the 255 vertex limit?
                unsigned changeOverComponent= 0;
                unsigned changeOverVerticesOffset = 0;
                for (unsigned int j = 0; j < nComponents; j++)
                {
                    datbuf->Skip(3);
                    unsigned v = datbuf->GetUint8();
                    if (v != prevV)
                    {
                        nVertices += v;
                        changeOverComponent = j;
                        changeOverVerticesOffset = prevV;
                        prevV = v;
                    }
#ifdef DEBUG
                    std::cout << "v: " << v << " vert: " <<
                        nVertices << " p: " << prevV << std::endl;
#endif
                    datbuf->Skip(10);
                }

                if (   item->entityType == 0x0
                    || item->entityType == 0x1
                    || item->entityType == 0x2
                    || item->entityType == 0x3
                    || item->entityType == 0x4
                    || item->entityType == 0x5
                    || item->entityType == 0x6
                    || item->entityType == 0x7
                    || item->entityType == 0x8
                    || item->entityType == 0x9
                    || item->entityType == 0xa
                    || item->entityType == 0xe
                    || item->entityType == 0xf
                    || item->entityType == 0x12
                    || item->entityType == 0x14
                    || item->entityType == 0x17
                    || item->entityType == 0x24
                    || item->entityType == 0x26
                    || item->entityType == 0x27
                    )
                {
                    if (nVertices > 0)
                        nVertices -= 1;
                }

#ifdef DEBUG
                std::cout << "Finished Blocks: vertts: " << nVertices << std::endl;
#endif
                for (unsigned int j = 0; j <= nVertices; j++)
                {
                    if (nVertices == 0) continue;
                    int x = datbuf->GetSint16LE();
                    int y = datbuf->GetSint16LE();
                    int z = datbuf->GetSint16LE();
                    item->vertices.push_back(new Vector3D(x, y, z));
                }

                if (nVertices == 0)
                {
                    if ((item->entityFlags & EF_UNBOUNDED) && (item->entityFlags & EF_2D_OBJECT) && (nComponents == 1))
                    {
                        datbuf->Skip(2);
                        item->sprite = datbuf->GetUint16LE();
                        datbuf->Skip(4);
                    }
                    else
                    {
                        item->sprite = -1;
                    }
#ifdef DEBUG
                    std::cout << "Sprite Object" << item->sprite << std::endl;
#endif
                }
                else
                {
                    // nComponents - 1 ? This doesn't seem right... something a little wrong with house?
                    if (item->entityType == 0xa) nComponents -= 1;
                    for (unsigned int j = 0; j < nComponents; j++)
                    {
#ifdef DEBUG
                        std::cout << "ComponentN: " << j << std::endl;
#endif
                        std::vector<std::uint16_t> nFaces{};
                        // There can be multiple "components" 
                        while (datbuf->GetUint16LE() == 0)
                        {
                            nFaces.emplace_back(datbuf->GetUint16LE());
                            datbuf->Skip(4); // Offset?
                        }
                        datbuf->Skip(-2); // Go back 
#ifdef DEBUG
                        std::cout << "Faces: " << nFaces << std::endl;
#endif
                        if (nFaces.empty())
                        {
                            continue;
                        }

                        assert(!nFaces.empty());

                        for (const auto faces : nFaces)
                        {
                            for (unsigned k = 0; k < faces; k++)
                            {
                                item->paletteSources.push_back(datbuf->GetUint8());
#ifdef DEBUG
                                std::cout << "Using palette: " << +item->paletteSources.back();
#endif
                                // There are four colours defined face color, edge color, ??, and ??
                                for (unsigned c = 0; c < 4; c++)
                                {
                                    if (c == 0)
                                    {
                                        auto color = datbuf->GetUint8();
#ifdef DEBUG
                                        std::cout << " color: " << +color;
#endif
                                        item->faceColors.push_back(color); 
                                    }
                                    // for now we ignore the edge colors and other colors
                                    else
                                    {
                                        auto color = datbuf->GetUint8();
                                        std::cout << " " << +color;
                                    }
                                }
                                std::cout << std::endl;
                                datbuf->Skip(3); // Offset?
                            }
                            for (unsigned k = 0; k < faces; k++)
                            {
                                unsigned vertI;
                                std::vector<std::uint16_t> vertIndices;
                                while ((vertI = datbuf->GetUint8()) != 0xff)
                                {
                                    // If we are at or beyond the component where the 
                                    // vertices change over then we need to add the respective offset
                                    if (j >= changeOverComponent)
                                        vertI += changeOverVerticesOffset;
                                    vertIndices.emplace_back(vertI);
                                }
                                item->faces.push_back(vertIndices);
                            }
                        }
                    }
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
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("TableResource::Save");
        throw;
    }
}
