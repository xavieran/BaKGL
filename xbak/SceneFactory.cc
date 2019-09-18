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

#include "PatternPolygonObject.h"
#include "SceneFactory.h"
#include "SolidPolygonObject.h"
#include "SpriteObject.h"

SceneFactory::SceneFactory(Zone &z)
    : zone(z)
{
}

SceneFactory::~SceneFactory()
{
}

void SceneFactory::AddFixedObjects(Scene* scene)
{
    TableResource *table = zone.GetTable();
    if (scene && table)
    {
        for (unsigned int i = 0; i < table->GetMapSize(); i++)
        {
            DatInfo *dat = table->GetDatItem(i);
            switch (dat->entityType)
            {
                case ET_EXTERIOR:
                    {
/*                        PatternPolygonObject *terrObj = new PatternPolygonObject();
                        for (unsigned j = 0; j < dat->vertices.size(); j++)
                        {
                            terrObj->AddVertex(Vertex(*(dat->vertices[j])));
                        }
                        scene->AddObject(Vector2D(x, y), terrObj);*/
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void SceneFactory::AddTiledObjects(Scene* scene, unsigned int x, unsigned int y, Image* /*terrainTexture*/)
{
    TableResource *table = zone.GetTable();
    TileWorldResource *tile = zone.GetTile(x, y);
    if (scene && table && tile)
    {
        for (unsigned int i = 0; i < tile->GetSize(); i++)
        {
            TileWorldItem item = tile->GetItem(i);
            DatInfo *dat = table->GetDatItem(item.type);
            switch (dat->entityType)
            {
                case ET_TERRAIN:
                    // terrain tiles are not required and will only slow things down
                    break;
                case ET_HILL:
                    {
                        SolidPolygonObject *spObj = new SolidPolygonObject(Vector2D(item.xloc + dat->max.GetX() / 2, item.yloc), 218);
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, dat->max.GetZ() - dat->min.GetZ())));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->max.GetX(), dat->max.GetY(), 0)));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->max.GetX(), dat->min.GetY(), 0)));
                        scene->AddObject(Vector2D(x, y), spObj);

                        spObj = new SolidPolygonObject(Vector2D(item.xloc, item.yloc + dat->min.GetY() / 2), 219);
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, dat->max.GetZ() - dat->min.GetZ())));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->max.GetX(), dat->min.GetY(), 0)));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->min.GetX(), dat->min.GetY(), 0)));
                        scene->AddObject(Vector2D(x, y), spObj);

                        spObj = new SolidPolygonObject(Vector2D(item.xloc + dat->min.GetX() / 2, item.yloc), 220);
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, dat->max.GetZ() - dat->min.GetZ())));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->min.GetX(), dat->min.GetY(), 0)));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->min.GetX(), dat->max.GetY(), 0)));
                        scene->AddObject(Vector2D(x, y), spObj);

                        spObj = new SolidPolygonObject(Vector2D(item.xloc, item.yloc + dat->max.GetY() / 2), 221);
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, dat->max.GetZ() - dat->min.GetZ())));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->min.GetX(), dat->max.GetY(), 0)));
                        spObj->AddVertex(Vertex(Vector3D(item.xloc, item.yloc, 0) + Vector3D(dat->max.GetX(), dat->max.GetY(), 0)));
                        scene->AddObject(Vector2D(x, y), spObj);
                    }
                    break;
                case ET_TREE:
                case ET_TOMBSTONE:
                case ET_SIGN:
                case ET_DEADBODY2:
                case ET_DIRTPILE:
                case ET_FIRE:
                case ET_FERN:
                case ET_ROCKPILE:
                case ET_BUSH1:
                case ET_BUSH2:
                case ET_BUSH3:
                case ET_SLAB:
                case ET_STUMP:
                case ET_WELL:
                case ET_ENGINE:
                case ET_SCARECROW:
                case ET_TRAP:
                case ET_COLUMN:
                case ET_BAG:
                case ET_LADDER:
                    scene->AddObject(Vector2D(x, y), new SpriteObject(Vector2D(item.xloc, item.yloc), zone.GetSprite(dat->sprite)));
                    break;
                default:
                    break;
            }
        }
    }
}

Scene * SceneFactory::CreateScene()
{
    std::vector<Image *> horizonImages;
    horizonImages.push_back(zone.GetHorizon(3));
    horizonImages.push_back(zone.GetHorizon(0));
    horizonImages.push_back(zone.GetHorizon(1));
    horizonImages.push_back(zone.GetHorizon(2));
    horizonImages.push_back(zone.GetHorizon(3));
    horizonImages.push_back(zone.GetHorizon(0));
    Image *horizonTexture = new Image(zone.GetHorizon(0)->GetWidth() * horizonImages.size(), zone.GetHorizon(0)->GetHeight(), horizonImages);
    std::vector<Image *> terrainImages;
    Image terrain1(zone.GetTerrain()->GetWidth(), zone.GetTerrain()->GetHeight() - 2, zone.GetTerrain()->GetPixels());
    terrainImages.push_back(&terrain1);
    Image terrain2(zone.GetTerrain()->GetWidth(), zone.GetTerrain()->GetHeight() - 2, zone.GetTerrain()->GetPixels() + zone.GetTerrain()->GetWidth());
    terrainImages.push_back(&terrain2);
    Image terrain3(zone.GetTerrain()->GetWidth(), zone.GetTerrain()->GetHeight() - 2, zone.GetTerrain()->GetPixels() + 2 * zone.GetTerrain()->GetWidth());
    terrainImages.push_back(&terrain3);
    Image *terrainTexture = new Image(zone.GetTerrain()->GetWidth() * terrainImages.size(), zone.GetTerrain()->GetHeight() - 2, terrainImages);
    Scene *scene = new Scene(horizonTexture, terrainTexture);
    AddFixedObjects(scene);
    for (unsigned int y = 1; y <= MAX_TILES; y++)
    {
        for (unsigned int x = 1; x <= MAX_TILES; x++)
        {
            AddTiledObjects(scene, x, y, terrainTexture);
        }
    }
    return scene;
}
