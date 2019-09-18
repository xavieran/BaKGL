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

/*! \file dumpads.cc
    \brief Main dumpads source file.

    This file contains the the main function of the .ADS data dumper.
 */

#include <iostream>

#include "AnimationResource.h"
#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <ADS-file>" << std::endl;
            return 1;
        }
        AnimationResource *anim = new AnimationResource;
        FileManager::GetInstance()->Load(anim, argv[1]);
        AnimationData data = anim->GetAnimationData(1);
        printf("%s %s %s\n", anim->GetVersion().c_str(), data.name.c_str(), data.resource.c_str());
        FileBuffer *scr = anim->GetScript();
        while (!scr->AtEnd())
        {
            unsigned int code = scr->GetUint16LE();
            printf("%04x ", code);
            switch (code)
            {
            case 0x0001:
                break;
            case 0x1030:
                printf(" %d %d", scr->GetUint16LE(), scr->GetUint16LE());
                break;
            case 0x1330:
                printf(" %d %d", scr->GetUint16LE(), scr->GetUint16LE());
                break;
            case 0x1350:
                printf(" %d %d", scr->GetUint16LE(), scr->GetUint16LE());
                break;
            case 0x1420:
                break;
            case 0x1510:
                break;
            case 0x1520:
                break;
            case 0x2005:
                printf(" %d %d %d %d", scr->GetUint16LE(), scr->GetUint16LE(), scr->GetUint16LE(), scr->GetUint16LE());
                break;
            case 0x2010:
                printf(" %d %d %d", scr->GetUint16LE(), scr->GetUint16LE(), scr->GetUint16LE());
                break;
            case 0xffff:
                break;
            default:
                printf(" unknown");
                break;
            }
            printf("\n");
        }
        delete anim;
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

