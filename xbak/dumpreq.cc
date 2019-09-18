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

/*! \file dumpreq.cc
    \brief Main dumpreq source file.

    This file contains the the main function of the REQ_*.DAT data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "RequestResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <REQ-file>" << std::endl;
            return 1;
        }
        RequestResource *req = new RequestResource;
        FileManager::GetInstance()->Load(req, argv[1]);
        printf("%5s %3d %3d %3d %3d %3d %3d\n", (req->IsPopup() ? "true" : "false"),
               req->GetRectangle().GetXPos(), req->GetRectangle().GetYPos(), req->GetRectangle().GetWidth(), req->GetRectangle().GetHeight(),
               req->GetXOff(), req->GetYOff());
        for (unsigned int i = 0; i < req->GetSize(); i++)
        {
            RequestData data = req->GetRequestData(i);
            printf("%3d: %3d %3d %3d %3d %3d %3d %3d %3d %3d %5s %s\n", i,
                   data.widget, data.action, data.xpos, data.ypos, data.width, data.height,
                   data.teleport, data.image, data.group, (data.visible ? "true" : "false"), data.label.c_str());
        }
        delete req;
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

