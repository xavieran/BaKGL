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

#include "Exception.h"
#include "PartyResource.h"

PartyResource::PartyResource()
        : data()
{}

PartyResource::~PartyResource()
{
    Clear();
}

unsigned int
PartyResource::GetSize() const
{
    return data.size();
}

PartyData *
PartyResource::GetData(const unsigned int n)
{
    return data[n];
}

void
PartyResource::Clear()
{
    for (unsigned int i = 0; i < data.size(); i++)
    {
        delete data[i];
    }
    data.clear();
}

void
PartyResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        unsigned int offset[PARTY_SIZE];
        for (unsigned int i = 0; i < PARTY_SIZE; i++)
        {
            offset[i] = buffer->GetUint16LE();
            buffer->Skip(93);
        }
        buffer->Skip(2);
        unsigned int start = buffer->GetBytesDone();
        for (unsigned int i = 0; i < PARTY_SIZE; i++)
        {
            buffer->Seek(start + offset[i]);
            PartyData *pd = new PartyData;
            pd->name = buffer->GetString();
            data.push_back(pd);
        }
    }
    catch (Exception &e)
    {
        e.Print("PartyResource::Load");
        throw;
    }
}

unsigned int
PartyResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("PartyResource::Save");
        throw;
    }
}
