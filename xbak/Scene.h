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

#ifndef SCENE_H
#define SCENE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <map>

#include "GenericObject.h"
#include "Image.h"
#include "Video.h"

class Scene
{
    private:
        Video *video;
        Image *horizonTexture;
        Image *terrainTexture;
        std::multimap<const Vector2D, GenericObject *> objects;
        std::multimap<const unsigned int, GenericObject *> zBuffer;
        void FillZBuffer ( Camera *cam );
        void DrawHorizon ( const int x, const int y, const int w, const int h, Camera *cam );
        void DrawGround ( const int x, const int y, const int w, const int h, Camera *cam );
        void DrawZBuffer ( const int x, const int y, const int w, const int h, Camera *cam );
    public:
        Scene ( Image *horizon, Image *terrain );
        ~Scene();
        void AddObject ( const Vector2D &cell, GenericObject *obj );
        void DrawFirstPerson ( const int x, const int y, const int w, const int h, Camera *cam );
        void DrawTopDown();
};

#endif
