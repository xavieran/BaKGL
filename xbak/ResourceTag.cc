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
#include "ResourceTag.h"

ResourceTag::ResourceTag()
{}

ResourceTag::~ResourceTag()
{
    tagMap.clear();
}

void
ResourceTag::Load(FileBuffer *buffer)
{
    try
    {
        unsigned int n = buffer->GetUint16LE();
        for (unsigned int i = 0; i < n; i++)
        {
            unsigned int id = buffer->GetUint16LE();
            std::string name = buffer->GetString();
            tagMap.insert(std::pair<const unsigned int, std::string>(id, name));
        }
    }
    catch (Exception &e)
    {
        e.Print("ResourceTag::Load");
    }
}

bool
ResourceTag::Find(const unsigned int id, std::string &name)
{
    try
    {
        name = tagMap[id];
    }
    catch (...)
    {
        return false;
    }
    return true;
}

