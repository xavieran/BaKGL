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
#include "ResourceIndex.h"

ResourceIndex::ResourceIndex()
        : resourceFilename("")
        , numResources(0)
        , resIdxMap()
        , resIdxIterator(resIdxMap.begin())
{
    
    
}

ResourceIndex::~ResourceIndex()
{
    resIdxMap.clear();
}

void ResourceIndex::Load(const std::string &filename)
{
    try
    {
        ResourceFile rmf;
        rmf.Open(filename, false);
        FileBuffer rmfBuffer(rmf.Size());
        rmf.Seek(0);
        rmf.Load(rmfBuffer);
        rmf.Close();
        if ((rmfBuffer.GetUint32LE() != 1) || (rmfBuffer.GetUint16LE() != 4))
        {
            throw DataCorruption(__FILE__, __LINE__);
        }
        resourceFilename = rmfBuffer.GetString(RES_FILENAME_LEN);
        numResources = rmfBuffer.GetUint16LE();

        ResourceFile res;
        res.Open(resourceFilename, false);
        FileBuffer resBuffer(RES_FILENAME_LEN + 4);
        for (unsigned int i = 0; i < numResources; i++)
        {
            unsigned int hashkey = rmfBuffer.GetUint32LE();
            std::streamoff offset = rmfBuffer.GetUint32LE();
            res.Seek(offset);
            res.Load(resBuffer);
            std::string resIdxName = resBuffer.GetString(RES_FILENAME_LEN);
            ResourceIndexData resIdxData;
            resIdxData.hashkey = hashkey;
            resIdxData.offset = offset + RES_FILENAME_LEN + 4;
            resIdxData.size = resBuffer.GetUint32LE();
            resIdxMap.insert(std::pair<const std::string, ResourceIndexData>(resIdxName, resIdxData));
        }
        res.Close();
    }
    catch (Exception &e)
    {
        e.Print("ResourceIndex::Load");
        throw;
    }

    for (resIdxIterator = resIdxMap.begin(); resIdxIterator != resIdxMap.end(); resIdxIterator++)
    {
        printf("%s\n", (*resIdxIterator).first.c_str());
    }
}

void ResourceIndex::Save(const std::string &filename)
{
    try
    {
        FileBuffer rmfBuffer(4 + 2 + RES_FILENAME_LEN + 2 + numResources * (4 + 4));
        rmfBuffer.PutUint32LE(1);
        rmfBuffer.PutUint16LE(4);
        rmfBuffer.PutString(resourceFilename, RES_FILENAME_LEN);
        rmfBuffer.PutUint16LE(numResources);

        ResourceFile res;
        res.Open(resourceFilename, false);
        FileBuffer resBuffer(RES_FILENAME_LEN + 4);
        unsigned int offset = 0;
        for (unsigned int i = 0; i < numResources; i++)
        {
            res.Seek(offset);
            res.Load(resBuffer);
            std::string resIdxName = resBuffer.GetString(RES_FILENAME_LEN);
            ResourceIndexData resIdxData;
            Find(resIdxName, resIdxData);
            rmfBuffer.PutUint32LE(resIdxData.hashkey);
            rmfBuffer.PutUint32LE(offset);
            offset += RES_FILENAME_LEN + 4 + resIdxData.size;
        }
        res.Close();

        ResourceFile rmf;
        rmf.Open(filename, true);
        rmf.Save(rmfBuffer);
        rmf.Close();
    }
    catch (Exception &e)
    {
        e.Print("ResourceIndex::Save");
        throw;
    }
}

std::string ResourceIndex::GetResourceFilename() const
{
    return resourceFilename;
}

unsigned int ResourceIndex::GetNumResources() const
{
    return numResources;
}

bool ResourceIndex::Find(const std::string &name, ResourceIndexData &data)
{
    std::map<const std::string, ResourceIndexData>::iterator it = resIdxMap.find(name);
    if (it != resIdxMap.end())
    {
        data = it->second;
        return true;
    }
    return false;
}

bool ResourceIndex::GetFirst(std::string& name, ResourceIndexData &data)
{
    resIdxIterator = resIdxMap.begin();
    if (resIdxIterator != resIdxMap.end())
    {
        name = resIdxIterator->first;
        data = resIdxIterator->second;
        return true;
    }
    return false;
}


bool ResourceIndex::GetNext(std::string& name, ResourceIndexData &data)
{
    if (resIdxIterator == resIdxMap.end())
    {
        return false;
    }
    resIdxIterator++;
    if (resIdxIterator != resIdxMap.end())
    {
        name = resIdxIterator->first;
        data = resIdxIterator->second;
        return true;
    }
    return false;
}
