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

/*! \file dumpttm.cc
    \brief Main dumpttm source file.

    This file contains the the main function of the .TTM data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "MovieResource.h"

int main ( int argc, char *argv[] )
{
    try
    {
        if ( argc != 2 )
        {
            std::cerr << "Usage: " << argv[0] << " <TTM-file>" << std::endl;
            return 1;
        }
        MovieResource *ttm = new MovieResource;
        FileManager::GetInstance()->Load ( ttm, argv[1] );
        printf ( "%s %d\n", ttm->GetVersion().c_str(), ttm->GetPages() );
        std::vector<MovieChunk *> mc = ttm->GetMovieChunks();
        for ( unsigned int i = 0; i < mc.size(); i++ )
        {
            printf ( "%4d %04x %-39s: ", i, mc[i]->code, mc[i]->name.c_str() );
            for ( unsigned int j = 0; j < mc[i]->data.size(); j++ )
            {
                printf ( " %4d", mc[i]->data[j] );
            }
            for ( unsigned int j = mc[i]->data.size(); j < 8; j++ )
            {
                printf ( "     " );
            }
            switch ( mc[i]->code )
            {
            case 0x0020:
                printf ( " save background" );
                break;
            case 0x0080:
                printf ( " draw background" );
                break;
            case 0x00c0:
                break;
            case 0x0110:
                printf ( " purge saved images" );
                break;
            case 0x0400:
                break;
            case 0x0500:
                break;
            case 0x0510:
                break;
            case 0x0ff0:
                printf ( " update" );
                break;
            case 0x1020:
                printf ( " delay (delay)" );
                break;
            case 0x1050:
                printf ( " select image (image)" );
                break;
            case 0x1060:
                printf ( " select palette (palette)" );
                break;
            case 0x1070:
                break;
            case 0x1100:
                break;
            case 0x1110:
                printf ( " set scene (scene)" );
                break;
            case 0x1120:
                break;
            case 0x1200:
                break;
            case 0x2000:
                printf ( " set frame (?, frame)" );
                break;
            case 0x2010:
                printf ( " set frame (?, frame)" );
                break;
            case 0x2300:
                break;
            case 0x2310:
                break;
            case 0x2320:
                break;
            case 0x2400:
                break;
            case 0x4000:
                printf ( " set window (x, y, w, h)" );
                break;
            case 0x4110:
                printf ( " fade out (first, n, steps, delay)" );
                break;
            case 0x4120:
                printf ( " fade in (first, n, steps, delay)" );
                break;
            case 0x4200:
                printf ( " save image (x, y, w, h)" );
                break;
            case 0x4210:
                printf ( " save image (x, y, w, h)" );
                break;
            case 0xa010:
                break;
            case 0xa030:
                break;
            case 0xa090:
                break;
            case 0xa0b0:
                break;
            case 0xa100:
                printf ( " set window (x, y, w, h)" );
                break;
            case 0xa500:
                printf ( " draw sprite (x, y, frame, image)" );
                break;
            case 0xa510:
                printf ( " draw sprite (x, y, frame, image)" );
                break;
            case 0xa520:
                printf ( " draw sprite (x, y, frame, image)" );
                break;
            case 0xa530:
                printf ( " draw sprite (x, y, frame, image)" );
                break;
            case 0xa5a0:
                break;
            case 0xa600:
                break;
            case 0xb600:
                printf ( " draw screen (x, y, w, h, ?, ?)" );
                break;
            case 0xc020:
                printf ( " load sound resource" );
                break;
            case 0xc030:
                printf ( " select sound (sound)" );
                break;
            case 0xc040:
                printf ( " deselect sound (sound)" );
                break;
            case 0xc050:
                printf ( " play sound (sound)" );
                break;
            case 0xc060:
                printf ( " stop sound (sound)" );
                break;
            case 0xf010:
                printf ( " load screen resource" );
                break;
            case 0xf020:
                printf ( " load image resource" );
                break;
            case 0xf040:
                break;
            case 0xf050:
                printf ( " load palette resource" );
                break;
            default:
                printf ( " unknown" );
                break;
            }
            printf ( "\n" );
        }
        delete ttm;
        FileManager::CleanUp();
        Directories::CleanUp();
    }
    catch ( Exception &e )
    {
        e.Print ( "main" );
    }
    catch ( ... )
    {
        /* every exception should have been handled before */
        std::cerr << "Unhandled exception" << std::endl;
    }
    return 0;
}

