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

/*! \file dumpbok.cc
    \brief Main dumpbok source file.

    This file contains the the main function of the .BOK data dumper.
 */

#include <iostream>

#include "BookResource.h"
#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <BOK-file>" << std::endl;
            return 1;
        }
        BookResource *bok = new BookResource;
        FileManager::GetInstance()->Load(bok, argv[1]);
        for (unsigned int i = 0; i < bok->GetSize(); i++)
        {
            PageData pd = bok->GetPage(i);
            printf("%d: %3d %3d %3d %3d %2d %2d %2d %2d %04x %1d\n", i,
                   pd.xpos, pd.ypos, pd.width, pd.height, pd.number, pd.id, pd.prevId, pd.nextId, pd.flag, pd.showNumber);
            for (unsigned int j = 0; j < pd.decorations.size(); j++)
            {
                printf("Decoration %2d: %3d %3d %3d %02x\n", j, pd.decorations[j].xpos, pd.decorations[j].ypos, pd.decorations[j].id, pd.decorations[j].flag);
            }
            for (unsigned int j = 0; j < pd.firstLetters.size(); j++)
            {
                printf("FirstLetter %2d: %3d %3d %3d %02x\n", j, pd.firstLetters[j].xpos, pd.firstLetters[j].ypos, pd.firstLetters[j].id, pd.firstLetters[j].flag);
            }
            for (unsigned int j = 0; j < pd.textBlocks.size(); j++)
            {
                printf("Text %2d: %1d %s\n", j, pd.textBlocks[j].italic, pd.textBlocks[j].txt.c_str());
            }
        }
        delete bok;
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

