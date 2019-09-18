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

/*! \file dumpgam.cc
    \brief Main dumpgam source file.

    This file contains the the main function of the saved game data dumper.
 */

#include <iostream>

#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "GameResource.h"
#include "ObjectResource.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <GAM-file>" << std::endl;
            return 1;
        }
        GameResource *gam = new GameResource;
        FileManager::GetInstance()->Load(gam, argv[1]);
        Game *game = gam->GetGame();
        printf("%s  l: (%d, %d) z: %d  c: (%d, %d)  p: (%d, %d)  o: %d\n",
               game->GetName().c_str(), gam->GetXLoc(), gam->GetYLoc(), gam->GetZone(),
               game->GetCamera()->GetPosition().GetCell().GetX(), game->GetCamera()->GetPosition().GetCell().GetY(),
               game->GetCamera()->GetPos().GetX(), game->GetCamera()->GetPos().GetY(), game->GetCamera()->GetHeading());
        for (unsigned int m = 0; m < game->GetParty()->GetNumMembers(); m++)
        {
            printf("%-10s:", game->GetParty()->GetMember(m)->GetName().c_str());
            for (unsigned int i = 0; i < NUM_STATS; i++)
            {
                printf("\n\t");
                for (unsigned int j = 0; j < NUM_STAT_VALUES; j++)
                {
                    printf(" %3d", game->GetParty()->GetMember(m)->GetStatistics().Get(i, j));
                }
            }
            printf("\n");
            Inventory *inv = game->GetParty()->GetMember(m)->GetInventory();
            for (unsigned int i = 0; i < inv->GetSize(); i++)
            {
                const InventoryItem *item = inv->GetItem(i);
                printf("\t%-24s %3d\n", ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).name.c_str(), item->GetValue());
            }
        }
        printf("active:");
        for (unsigned int i = 0; i < game->GetParty()->GetNumActiveMembers(); i++)
        {
            printf(" %s", game->GetParty()->GetActiveMember(i)->GetName().c_str());
        }
        printf("\n");
        delete gam;
        ObjectResource::CleanUp();
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

