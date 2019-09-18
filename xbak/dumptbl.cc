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

/*! \file dumptbl.cc
    \brief Main dumptbl source file.

    This file contains the the main function of the .TBL data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "TableResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <TBL-file>" << std::endl;
            return 1;
        }
        TableResource *tbl = new TableResource;
        FileManager::GetInstance()->Load(tbl, argv[1]);
        for (unsigned int i = 0; i < tbl->GetMapSize(); i++)
        {
            GidInfo *gid = tbl->GetGidItem(i);
            DatInfo *dat = tbl->GetDatItem(i);
            printf("%3d: %-8s (%6d %6d) %04x %02x %2d %2d %2d %2d\n",
                   i, tbl->GetMapItem(i).c_str(), gid->xradius, gid->yradius, gid->flags,
                   dat->entityFlags, dat->entityType, dat->terrainType, dat->terrainClass, dat->sprite);
            printf("\t(%6d, %6d, %6d)  (%6d, %6d, %6d)  (%6d, %6d, %6d)\n",
                   dat->min.GetX(), dat->min.GetY(), dat->min.GetZ(),
                   dat->max.GetX(), dat->max.GetY(), dat->max.GetZ(),
                   dat->pos.GetX(), dat->pos.GetY(), dat->pos.GetZ());
            for (unsigned int j = 0; j < gid->textureCoords.size(); j++)
            {
                printf("\t\t(%4d, %4d)\n",
                       gid->textureCoords[j]->GetX(), gid->textureCoords[j]->GetY());
            }
            for (unsigned int j = 0; j < gid->otherCoords.size(); j++)
            {
                printf("\t\t(%6d, %6d)\n",
                       gid->otherCoords[j]->GetX(), gid->otherCoords[j]->GetY());
            }
            for (unsigned int j = 0; j < dat->vertices.size(); j++)
            {
                printf("\t\t(%6d, %6d, %6d)\n",
                       dat->vertices[j]->GetX(), dat->vertices[j]->GetY(), dat->vertices[j]->GetZ());
            }
        }
        delete tbl;
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

