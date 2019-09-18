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
#include "ResourceArchive.h"

ResourceArchive::ResourceArchive()
        : ResourceFile()
{}

ResourceArchive::~ResourceArchive()
{}

void
ResourceArchive::LoadResource(FileBuffer &buffer, const std::streamoff offset)
{
    try
    {
        Seek(offset);
        Load(buffer);
    }
    catch (Exception &e)
    {
        e.Print("ResourceArchive::LoadResource");
        throw;
    }
}

