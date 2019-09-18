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

#ifndef RESOURCE_INDEX_H
#define RESOURCE_INDEX_H

#include <map>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FileBuffer.h"

struct ResourceIndexData
{
    unsigned int hashkey;
    std::streamoff offset;
    unsigned int size;
};

class ResourceIndex
{
private:
    std::string resourceFilename;
    unsigned int numResources;
    std::map <const std::string, ResourceIndexData> resIdxMap;
    std::map<const std::string, ResourceIndexData>::iterator resIdxIterator;
public:
    ResourceIndex();
    virtual ~ResourceIndex();
    void Load ( const std::string &filename );
    void Save ( const std::string &filename );
    std::string GetResourceFilename() const;
    unsigned int GetNumResources() const;
    bool Find ( const std::string &name, ResourceIndexData &data );
    bool GetFirst ( std::string& name, ResourceIndexData &data );
    bool GetNext ( std::string& name, ResourceIndexData &data );
};

#endif

