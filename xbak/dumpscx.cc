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

/*! \file dumpscx.cc
    \brief Main dumpscx source file.

    This file contains the the main function of the .SCX data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "ScreenResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <SCX-file>" << std::endl;
            return 1;
        }
        ScreenResource *scx = new ScreenResource;
        FileManager::GetInstance()->Load(scx, argv[1]);
        Image *image = scx->GetImage();
        printf("%dx%d\n", image->GetWidth(), image->GetHeight());
        for (int y = 0; y < image->GetHeight(); y++)
        {
            for (int x = 0; x < image->GetWidth(); x++)
            {
                printf("%02x ", image->GetPixel(x, y));
            }
            printf("\n");
        }
        delete scx;
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

