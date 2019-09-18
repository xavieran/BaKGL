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

/*! \file dumpbmx.cc
    \brief Main dumpbmx source file.

    This file contains the the main function of the .BMX data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "ImageResource.h"
#include "TaggedImageResource.h"

int main ( int argc, char *argv[] )
{
    try
    {
        if ( ! ( ( argc == 2 ) || ( ( argc == 3 ) && ( std::string ( argv[2] ) == "TAG" ) ) ) )
        {
            std::cerr << "Usage: " << argv[0] << " <BMX-file> [TAG]" << std::endl;
            return 1;
        }
        if ( argc == 2 )
        {
            ImageResource *bmx = new ImageResource;
            FileManager::GetInstance()->Load ( bmx, argv[1] );
            for ( unsigned int i = 0; i < bmx->GetNumImages(); i++ )
            {
                Image *image = bmx->GetImage ( i );
                printf ( "%2d  %dx%d\n", i, image->GetWidth(), image->GetHeight() );
                for ( int y = 0; y < image->GetHeight(); y++ )
                {
                    for ( int x = 0; x < image->GetWidth(); x++ )
                    {
                        printf ( "%02x ", image->GetPixel ( x, y ) );
                    }
                    printf ( "\n" );
                }
            }
            delete bmx;
            FileManager::CleanUp();
            Directories::CleanUp();
        }
        else
        {
            TaggedImageResource *bmx = new TaggedImageResource;
            FileManager::GetInstance()->Load ( bmx, argv[1] );
            for ( unsigned int i = 0; i < bmx->GetNumImages(); i++ )
            {
                Image *image = bmx->GetImage ( i );
                printf ( "%2d  %dx%d\n", i, image->GetWidth(), image->GetHeight() );
                for ( int y = 0; y < image->GetHeight(); y++ )
                {
                    for ( int x = 0; x < image->GetWidth(); x++ )
                    {
                        printf ( "%02x ", image->GetPixel ( x, y ) );
                    }
                    printf ( "\n" );
                }
            }
            delete bmx;
            FileManager::CleanUp();
            Directories::CleanUp();
        }
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

