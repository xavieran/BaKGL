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

/*! \file dumpfnt.cc
    \brief Main dumpfnt source file.

    This file contains the the main function of the .FNT data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "FontResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <FNT-file>" << std::endl;
            return 1;
        }
        FontResource *fnt = new FontResource;
        FileManager::GetInstance()->Load(fnt, argv[1]);
        Font *font = fnt->GetFont();
        for (unsigned int i = 0; i < font->GetSize(); i++)
        {
            printf("%2d: '%c' (%d)\n", i, i + font->GetFirst(), font->GetWidth(i));
            FontGlyph glyph = font->GetGlyph(i);
            for (int j = 0; j < font->GetHeight(); j++)
            {
                for (unsigned int k = 0; k < glyph.width; k++)
                {
                    printf("%c", glyph.data[j] & (0x8000 >> k) ? '*' : '.');
                }
                printf("\n");
            }
        }
        delete fnt;
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

