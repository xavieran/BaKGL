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

#include "MediaToolkit.h"
#include "Scene.h"

Scene::Scene(Image *horizon, Image *terrain)
        : video(MediaToolkit::GetInstance()->GetVideo())
        , horizonTexture(horizon)
        , terrainTexture(terrain)
        , objects()
        , zBuffer()
{
}

Scene::~Scene()
{
    for (std::multimap<const Vector2D, GenericObject *>::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        delete it->second;
    }
    objects.clear();
    zBuffer.clear();
    delete horizonTexture;
    delete terrainTexture;
}

void Scene::AddObject(const Vector2D &cell, GenericObject *obj)
{
    objects.insert(std::pair<const Vector2D, GenericObject *>(cell, obj));
}

void Scene::FillZBuffer(Camera *cam)
{
    zBuffer.clear();
    Vector2D cell = cam->GetPosition().GetCell();
    Vector2D pos = cam->GetPos();
    Angle angle = cam->GetAngle();
    for (std::multimap<const Vector2D, GenericObject *>::iterator it = objects.lower_bound(cell); it != objects.upper_bound(cell); ++it)
    {
        it->second->CalculateRelativePosition(pos);
        unsigned int distance;
        if (it->second->IsInView(angle, distance))
        {
            zBuffer.insert(std::pair<int, GenericObject *>(distance, it->second));
        }
    }
}

void Scene::DrawHorizon(const int x, const int y, const int w, const int, Camera *cam)
{
    const int HORIZON_TOP_SIZE = 34;
    video->FillRect(x, y, w, HORIZON_TOP_SIZE, horizonTexture->GetPixel(0, 0));
    video->FillRect(x, y + HORIZON_TOP_SIZE, w, horizonTexture->GetHeight(), horizonTexture->GetPixels(),
                    (cam->GetHeading() << 2) - x, -y - HORIZON_TOP_SIZE, horizonTexture->GetWidth());
}

void Scene::DrawGround(const int x, const int y, const int w, const int h, Camera *cam)
{
    const int TERRAIN_YOFFSET = 81;
    int offset = (((cam->GetHeading() * 16) + ((cam->GetPos().GetX() + cam->GetPos().GetY()) / 100)) % (terrainTexture->GetWidth() / 3));
    video->FillRect(x, y + h - TERRAIN_HEIGHT, w, TERRAIN_HEIGHT, terrainTexture->GetPixels(),
                    offset - x, TERRAIN_YOFFSET - y - h + TERRAIN_HEIGHT, terrainTexture->GetWidth());
}

void Scene::DrawZBuffer(const int x, const int y, const int w, const int h, Camera *cam)
{
    for (std::multimap<const unsigned int, GenericObject *>::reverse_iterator it = zBuffer.rbegin(); it != zBuffer.rend(); it++)
    {
        it->second->DrawFirstPerson(x, y, w, h, cam);
    }
}

void Scene::DrawFirstPerson(const int x, const int y, const int w, const int h, Camera *cam)
{
    FillZBuffer(cam);
    DrawGround(x, y, w, h, cam);
    DrawHorizon(x, y, w, h, cam);
    DrawZBuffer(x, y, w, h, cam);
}

void Scene::DrawTopDown()
{
}
