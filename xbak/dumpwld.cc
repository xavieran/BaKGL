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

/*! \file dumpwld.cc
    \brief Main dumpwld source file.

    This file contains the the main function of the .WLD data dumper.
 */

#include <cstring>
#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "TileWorldResource.h"

const unsigned int MAP_SIZE_X = 96;
const unsigned int MAP_SIZE_Y = 96;

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <WLD-file>" << std::endl;
            return 1;
        }
        TileWorldResource *wld = new TileWorldResource;
        FileManager::GetInstance()->Load(wld, argv[1]);
        printf("%d %d %d %d %d\n", wld->GetSize(), wld->GetMinX(), wld->GetMaxX(), wld->GetMinY(), wld->GetMaxY());
        unsigned int deltaX = wld->GetMaxX() - wld->GetMinX();
        unsigned int deltaY = wld->GetMaxY() - wld->GetMinY();
        uint8_t *map = new uint8_t[MAP_SIZE_X * MAP_SIZE_Y];
        memset(map, 0, MAP_SIZE_X * MAP_SIZE_Y);
        for (unsigned int i = 0; i < wld->GetSize(); i++)
        {
            TileWorldItem mi = wld->GetItem(i);
            printf("%6d,%6d: %3d (%02x) %08x\n", mi.xloc, mi.yloc, mi.type, mi.type, mi.flags);
            if (mi.type > 0)
            {
                unsigned int x = (mi.xloc - wld->GetMinX()) * MAP_SIZE_X / deltaX;
                unsigned int y = (mi.yloc - wld->GetMinY()) * MAP_SIZE_Y / deltaY;
                map[x + y * MAP_SIZE_X] = mi.type;
            }
        }
        for (unsigned int y = 0; y < MAP_SIZE_Y; y++)
        {
            for (unsigned int x = 0; x < MAP_SIZE_X; x++)
            {
                uint8_t m = map[x + y * MAP_SIZE_X];
                if (m > 0)
                {
                    printf("%02x", m);
                }
                else
                {
                    printf("--");
                }
            }
            printf("\n");
        }
        delete map;
        delete wld;
        FileManager::CleanUp();
        Directories::CleanUp();
    }
    catch (Exception &e)
    {
        e.Print("main");
    }
    catch (...)
    {
        /* every exception should have been handled before */
        std::cerr << "Unhandled exception" << std::endl;
    }
    return 0;
}

