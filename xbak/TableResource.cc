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

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

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
        std::cout << " NumMapItems: "  << numMapItems << std::endl;
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
            std::cout << "Load GIDOff: " << i << " name: " 
                << mapItems[i] << std::hex << " upper: " << upper
                << " lower: " << lower << " off: " << offset <<  " buf: " 
                << std::dec << gidbuf->Tell() << std::endl;
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
            std::cout << "GidItem: " << mapItems[i] << " " << i << " length: "
                << " i: " << std::hex << i << " length: " << std::dec << gidLength[i] << std::endl;
            gidbuf->Dump(gidLength[i]);
        }
    
        for (unsigned int i = 0; i < numMapItems; i++)
        {
            std::cout << " gidItem: " << mapItems[i] << " " << i << " current: " << gidbuf->Tell()
                << " off: " << gidOffset[i] << std::endl;
            // Seek a fixed distance from the start of the gidbuf
            gidbuf->Seek(gidOffset[i]); 
            GidInfo *item = new GidInfo();

            item->xradius = gidbuf->GetUint16LE();
            item->yradius = gidbuf->GetUint16LE();
            std::cout << "HitRad: (" << item->xradius << "," << item->yradius << ")"
                << std::endl;
            bool more = gidbuf->GetUint16LE() > 0;
            item->flags = gidbuf->GetUint16LE();
            // 86 -> Offset into giditem??
            // t010106
            // 00000000: 80 3e 80 3e 00 0f 14 00 | 6e 00 04 00 00 00 86 00 
            if (more)
            {
                gidbuf->Skip(2);
                unsigned int n = gidbuf->GetUint16LE();
                std::cout << "n: " << n << std::endl;
                gidbuf->Skip(2);
                for (unsigned int j = 0; j < n; j++)
                {
                    int u = gidbuf->GetSint8();
                    int v = gidbuf->GetSint8();
                    int x = gidbuf->GetSint16LE();
                    int y = gidbuf->GetSint16LE();
                    std::cout << std::hex << "uvxy " << u << " " << v << " " << x << " " << y << std::endl << std::dec;
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
            unsigned int lower = datbuf->GetUint16LE();
            unsigned int upper = datbuf->GetUint16LE();
            unsigned int offset = (upper << 4) + (lower & 0x000f);
            std::cout << "Load DatOff: " << i << " name: " 
                << mapItems[i] << std::hex << " upper: " << upper
                << " lower: " << lower << " off: " << offset <<  " buf: " 
                << std::dec << datbuf->Tell() << std::endl;

            datOffset[i] = offset;
        }

        for (unsigned int i = 0; i < (numMapItems - 1); i++)
        {
            datbuf->Seek(datOffset[i]);
            unsigned int length = datOffset[i + 1] - datOffset[i];
            std::cout << "DatItem: " << mapItems[i] << " " << i << " length: " << length
                << " i: " << std::hex << i << " length: " << length << std::dec << std::endl;
            datbuf->Dump(length);
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
            std::cout << "\tDatItem: " << mapItems[i]  << " 0x" << std::hex << i << " f: " << item->entityFlags
                << " et: " << item->entityType << " tt: " << item->terrainType
                << " tc: " << item->terrainClass << " more: " << more 
				<< std::dec << " "  << datbuf->Tell() << std::endl;
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
                unsigned int nPolys = datbuf->GetUint16LE();
                std::cout << "Polygons: " << nPolys << std::endl;

                datbuf->Skip(2); // Seems important...
				unsigned nVertices = 0;
				unsigned prevV = 0;
                for (unsigned int j = 0; j < nPolys; j++)
                {
					datbuf->Skip(3);
					unsigned v = datbuf->GetUint8();
					if (v != prevV)
					{
						nVertices += v;
						prevV = v;
					}
					std::cout << "v: " << v << " vert: " <<
						nVertices << " p: " << prevV << std::endl;
					datbuf->Skip(10);
                }

				if (item->entityType == 0x4
					|| item->entityType == 0x2
					|| item->entityType == 0x0
					|| item->entityType == 0x3
					|| item->entityType == 0x14
					|| item->entityType == 0xa
					|| item->entityType == 0x6
					|| item->entityType == 0x8
					|| item->entityType == 0x12
					|| item->entityType == 0x7)
					nVertices -= 1;

                std::cout << "Finished Blocks: vertts: " << nVertices << std::endl;
                for (unsigned int j = 0; j <= nVertices; j++)
                {
					if (nVertices == 0) std::cout << "No vertices" << std::endl;
					std::cout << j << " "
						<< datbuf->Tell() << std::endl;
					datbuf->Dump(6);
                    int x = datbuf->GetSint16LE();
                    int y = datbuf->GetSint16LE();
                    int z = datbuf->GetSint16LE();
                    item->vertices.push_back(new Vector3D(x, y, z));
                    std::cout << "xyz: " << x << " " << y << " " << z << " - " 
						<< datbuf->Tell() << std::endl;
                }
				std::cout << "Fininshe vertices" << std::endl;

				datbuf->Dump(8);
				if (nVertices == 0)
				{
					if ((item->entityFlags & EF_UNBOUNDED) && (item->entityFlags & EF_2D_OBJECT) && (nPolys == 1))
					{
						datbuf->Skip(2);
						item->sprite = datbuf->GetUint16LE();
						datbuf->Skip(4);
					}
					else
						item->sprite = -1;
					std::cout << "Sprite Object" << item->sprite << std::endl;
				}
				else
				{
					for (unsigned int j = 0; j < nPolys; j++)
					{
						datbuf->Skip(2); // Empty
						unsigned nFaces = datbuf->GetUint16LE();
						datbuf->Skip(4); // Offset?
						for (unsigned k = 0; k < nFaces; k++)
						{
							datbuf->Skip(1);
							for (unsigned c = 0; c < 4; c++)
							{
								datbuf->Skip(1); // color index
							}
							datbuf->Skip(3); // Offset?
						}
						for (unsigned k = 0; k < nFaces; k++)
						{
							unsigned vertI;
							std::vector<std::uint16_t> vertIndices;
							datbuf->Dump(5);
							while ((vertI = datbuf->GetUint8()) != 0xff)
							{
								vertIndices.emplace_back(vertI);
							}
							std::cout << "Face: " << vertIndices << std::endl;
							item->faces.push_back(vertIndices);
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
