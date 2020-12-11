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
#include <cassert>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "BMPWriter.h"
#include "ScreenResource.h"
#include "PaletteResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: " << argv[0] << " <SCX-file>" << " <PAL-file> <OUTPUT BMP>" << std::endl;
            return 1;
        }
        ScreenResource *scx = new ScreenResource;
        FileManager::GetInstance()->Load(scx, argv[1]);
        Image *image = scx->GetImage();

        PaletteResource *palette = new PaletteResource;
        FileManager::GetInstance()->Load(palette, argv[2]);

        std::ofstream out{};
        std::cout << "Writing to: " << argv[3] << std::endl;
        out.open(argv[3], std::ios::out | std::ios::binary);
        assert(out.good());

        WriteBMP(out, *image, *palette->GetPalette());

        out.close();

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

