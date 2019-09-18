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

/*! \file dumplbl.cc
    \brief Main dumplbl source file.

    This file contains the the main function of the LBL_*.DAT data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "LabelResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <LBL-file>" << std::endl;
            return 1;
        }
        LabelResource *lbl = new LabelResource;
        FileManager::GetInstance()->Load(lbl, argv[1]);
        for (unsigned int i = 0; i < lbl->GetSize(); i++)
        {
            LabelData data = lbl->GetLabelData(i);
            printf("%3d: %3d %3d %3d %3d %3d %s\n", i,
                   data.xpos, data.ypos, data.type, data.color, data.shadow, data.label.c_str());
        }
        delete lbl;
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

