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
#include "NameResource.h"

NameResource::NameResource()
        : name()
{}

NameResource::~NameResource()
{
    Clear();
}

unsigned int
NameResource::GetSize() const
{
    return name.size();
}

std::string
NameResource::GetName(unsigned int n) const
{
    return name[n];
}

void
NameResource::Clear()
{
    name.clear();
}

void
NameResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        unsigned int n = buffer->GetUint16LE();
        unsigned int *offset = new unsigned int[n];
        for (unsigned int i = 0; i < n; i++)
        {
            offset[i] = buffer->GetUint16LE();
        }
        buffer->Skip(2);
        unsigned int start = buffer->GetBytesDone();
        for (unsigned int i = 0; i < n; i++)
        {
            buffer->Seek(start + offset[i]);
            name.push_back(buffer->GetString());
        }
        delete[] offset;
    }
    catch (Exception &e)
    {
        e.Print("NameResource::Load");
        throw;
    }
}

unsigned int
NameResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("NameResource::Save");
        throw;
    }
}
