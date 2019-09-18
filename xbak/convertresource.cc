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

/*! \file convertresource.cc
    \brief Main convertresource source file.

    This file contains the the main function of the resource conversion utility.
    The resource converter can extract and recombine the individual data files
    from and to the resource archive. The resource meta file (.RMF) is created
    when archiving the individual data files. With the resource conversion utility
    it is also possible to extract and recombine individual bitmap images from and
    to .BMX data files. The images are stored in .BMP format.
 */

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "BMP.h"
#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "ImageResource.h"
#include "PaletteResource.h"

typedef enum _CommandType
{
    CT_UNKNOWN,
    CT_ARCHIVE,
    CT_EXTRACT
} CommandType;

typedef enum _FileType
{
    FT_UNKNOWN,
    FT_BMX,
    FT_RMF
} FileType;

CommandType
get_command_type(char *param)
{
    if (!param)
    {
        return CT_UNKNOWN;
    }
    if (strncmp(param, "A", 1) == 0)
    {
        return CT_ARCHIVE;
    }
    if (strncmp(param, "E", 1) == 0)
    {
        return CT_EXTRACT;
    }
    return CT_UNKNOWN;
}

FileType
get_file_type(char *param)
{
    if (!param)
    {
        return FT_UNKNOWN;
    }
    if (strncmp(param, "BMX", 3) == 0)
    {
        return FT_BMX;
    }
    if (strncmp(param, "RMF", 3) == 0)
    {
        return FT_RMF;
    }
    return FT_UNKNOWN;
}

int main(int argc, char **argv)
{
    try
    {
        CommandType ct = get_command_type(argv[1]);
        FileType ft = get_file_type(argv[2]);
        if ((ct == CT_UNKNOWN) || (ft == FT_UNKNOWN))
        {
            printf("Usage: %s <A|E> <RMF|BMX> [files...]\n", argv[0]);
            return -1;
        }
        switch (ft)
        {
        case FT_BMX:
        {
            if (argc != 5)
            {
                printf("Usage: %s <A|E> BMX <BMX-file> <PAL-file>\n", argv[0]);
                return -1;
            }
            ImageResource *bmx = new ImageResource;
            FileManager::GetInstance()->Load(bmx, argv[3]);
            PaletteResource *pal = new PaletteResource;
            FileManager::GetInstance()->Load(pal, argv[4]);
            BMP bmp;
            bmp.SetPalette(pal->GetPalette());
            for (unsigned int i = 0; i < bmx->GetNumImages(); i++)
            {
                bmp.SetImage(bmx->GetImage(i));
                std::stringstream ss;
                ss << argv[3] << std::setfill('0') << std::setw(2) << i << ".bmp";
                if (ct == CT_ARCHIVE)
                {
                    bmp.Load(ss.str());
                }
                else
                {
                    bmp.Save(ss.str());
                }
            }
            if (ct == CT_ARCHIVE)
            {
                FileManager::GetInstance()->Save(bmx, argv[3]);
            }
            delete pal;
            delete bmx;
        }
        break;
        case FT_RMF:
            if (argc != 3)
            {
                printf("Usage: %s <A|E> RMF\n", argv[0]);
                return -1;
            }
            if (ct == CT_ARCHIVE)
            {
                FileManager::GetInstance()->ArchiveAllResources();
            }
            else
            {
                FileManager::GetInstance()->ExtractAllResources();
            }
            break;
        default:
            break;
        }
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
