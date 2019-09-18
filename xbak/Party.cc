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

#include "Party.h"

Party::Party()
        : members()
        , numActiveMembers(0)
{}

Party::~Party()
{
    for (unsigned int i = 0; i < members.size(); i++)
    {
        delete members[i];
    }
    members.clear();
    numActiveMembers = 0;
}

unsigned int
Party::GetNumMembers() const
{
    return members.size();
}

PlayerCharacter *
Party::GetMember(const unsigned int n)
{
    return members[n];
}

unsigned int
Party::GetNumActiveMembers() const
{
    return numActiveMembers;
}

PlayerCharacter *
Party::GetActiveMember(const int order)
{
    unsigned int n = GetActiveMemberIndex(order);
    if (n < members.size())
    {
        return members[n];
    }
    return 0;
}

unsigned int
Party::GetActiveMemberIndex(const int order) const
{
    unsigned int i = 0;
    while (i < members.size())
    {
        if (order == members[i]->GetOrder())
        {
            return i;
        }
        i++;
    }
    return members.size();
}

PlayerCharacter *
Party::GetSelectedMember()
{
    unsigned int i = 0;
    while (i < members.size())
    {
        if (members[i]->IsSelected())
        {
            return members[i];
        }
        i++;
    }
    return 0;
}

void
Party::AddMember(PlayerCharacter *pc)
{
    members.push_back(pc);
}

void
Party::ActivateMember(const unsigned int n, const int order)
{
    PlayerCharacter *pc = GetActiveMember(order);
    if (pc)
    {
        pc->SetOrder(-1);
        numActiveMembers--;
    }
    if (members[n]->GetOrder() >= 0)
    {
        numActiveMembers--;
    }
    members[n]->SetOrder(order);
    numActiveMembers++;
}

void
Party::SelectMember(const int order)
{
    for (unsigned int i = 0; i < members.size(); i++)
    {
        members[i]->Select((order >= 0) && (members[i]->GetOrder() == order));
    }
}
