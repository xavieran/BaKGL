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

#include "ConfigFile.h"
#include "Exception.h"
#include "FileManager.h"
#include "GameFile.h"
#include "ResourceFile.h"

FileManager* FileManager::instance = 0;

FileManager::FileManager()
{
    resIndex.Load("krondor.rmf");
    resArchive.Open(resIndex.GetResourceFilename(), false);
}

FileManager::~FileManager()
{
    resArchive.Close();
}

FileManager*
FileManager::GetInstance()
{
    if (!instance)
    {
        instance = new FileManager();
    }
    return instance;
}

void
FileManager::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

FileBuffer*
FileManager::LoadConfig(const std::string &name)
{
    try
    {
        ConfigFile cfgfile;
        cfgfile.Open(name, false);
        FileBuffer *buffer = new FileBuffer(cfgfile.Size());
        cfgfile.Seek(0);
        cfgfile.Load(*buffer);
        cfgfile.Close();
        return buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::LoadConfig");
        throw;
    }
    return 0;
}

void
FileManager::SaveConfig(const std::string &name, FileBuffer* buffer)
{
    try
    {
        ConfigFile cfgfile;
        cfgfile.Open(name, true);
        cfgfile.Save(*buffer);
        cfgfile.Close();
    }
    catch (Exception &e)
    {
        e.Print("FileManager::SaveConfig");
        throw;
    }
}

void
FileManager::SaveConfig(const std::string &name, FileBuffer* buffer, const unsigned int n)
{
    try
    {
        ConfigFile cfgfile;
        cfgfile.Open(name, true);
        cfgfile.Save(*buffer, n);
        cfgfile.Close();
    }
    catch (Exception &e)
    {
        e.Print("FileManager::SaveConfig");
        throw;
    }
}

FileBuffer*
FileManager::LoadGame(const std::string &name)
{
    try
    {
        GameFile gamfile;
        gamfile.Open(name, false);
        FileBuffer *buffer = new FileBuffer(gamfile.Size());
        gamfile.Seek(0);
        gamfile.Load(*buffer);
        gamfile.Close();
        return buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::LoadGame");
        throw;
    }
    return 0;
}

void
FileManager::SaveGame(const std::string &name, FileBuffer* buffer)
{
    try
    {
        GameFile gamfile;
        gamfile.Open(name, true);
        gamfile.Save(*buffer);
        gamfile.Close();
    }
    catch (Exception &e)
    {
        e.Print("FileManager::SaveGame");
        throw;
    }
}

void
FileManager::SaveGame(const std::string &name, FileBuffer* buffer, const unsigned int n)
{
    try
    {
        GameFile gamfile;
        gamfile.Open(name, true);
        gamfile.Save(*buffer, n);
        gamfile.Close();
    }
    catch (Exception &e)
    {
        e.Print("FileManager::SaveGame");
        throw;
    }
}

FileBuffer*
FileManager::LoadResource(const std::string &name)
{
    try
    {
        ResourceFile resfile;
        resfile.Open(name, false);
        FileBuffer *buffer = new FileBuffer(resfile.Size());
        resfile.Seek(0);
        resfile.Load(*buffer);
        resfile.Close();
        return buffer;
    }
    catch (Exception &e1)
    {
        ResourceIndexData resIdxData = {0, 0, 0};
        if (resIndex.Find(name, resIdxData) && (resIdxData.size != 0))
        {
            try
            {
                FileBuffer *buffer = new FileBuffer(resIdxData.size);
                resArchive.LoadResource(*buffer, resIdxData.offset);
                return buffer;
            }
            catch (Exception &e2)
            {
                e2.Print("FileManager::LoadResource");
                throw;
            }
        }
        else
        {
            throw FileNotFound(__FILE__, __LINE__, name);
        }
    }
    return 0;
}

void
FileManager::SaveResource(const std::string &name, FileBuffer* buffer)
{
    try
    {
        ResourceFile resfile;
        resfile.Open(name, true);
        resfile.Save(*buffer);
        resfile.Close();
    }
    catch (Exception &e)
    {
        e.Print("FileManager::SaveResource");
        throw;
    }
}

void
FileManager::SaveResource(const std::string &name, FileBuffer* buffer, const unsigned int n)
{
    try
    {
        ResourceFile resfile;
        resfile.Open(name, true);
        resfile.Save(*buffer, n);
        resfile.Close();
    }
    catch (Exception &e)
    {
        e.Print("FileManager::SaveResource");
        throw;
    }
}

bool
FileManager::ConfigExists(const std::string &name)
{
    try
    {
        ConfigFile cfgfile;
        cfgfile.Open(name, false);
        cfgfile.Close();
        return true;
    }
    catch (Exception &e)
    {
        return false;
    }
    return false;
}

void
FileManager::Load(ConfigData *cfg, const std::string &name)
{
    try
    {
        FileBuffer *buffer;
        buffer = LoadConfig(name);
        cfg->Load(buffer);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::Load");
        throw;
    }
}

void
FileManager::Save(ConfigData *cfg, const std::string &name)
{
    try
    {
        FileBuffer *buffer = new FileBuffer(16);
        unsigned int size = cfg->Save(buffer);
        SaveConfig(name, buffer, size);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::Save");
        throw;
    }
}

bool
FileManager::GameExists(const std::string &name)
{
    try
    {
        GameFile gamfile;
        gamfile.Open(name, false);
        gamfile.Close();
        return true;
    }
    catch (Exception &e)
    {
        return false;
    }
    return false;
}

void
FileManager::Load(GameData *gam, const std::string &name)
{
    try
    {
        FileBuffer *buffer;
        buffer = LoadGame(name);
        gam->Load(buffer);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::Load");
        throw;
    }
}

void
FileManager::Save(GameData *gam, const std::string &name)
{
    try
    {
        FileBuffer *buffer = new FileBuffer(400000);
        unsigned int size = gam->Save(buffer);
        SaveGame(name, buffer, size);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::Save");
        throw;
    }
}

bool
FileManager::ResourceExists(const std::string &name)
{
    try
    {
        ResourceFile resfile;
        resfile.Open(name, false);
        resfile.Close();
        return true;
    }
    catch (Exception &e1)
    {
        ResourceIndexData resIdxData = {0, 0, 0};
        return (resIndex.Find(name, resIdxData) && (resIdxData.size != 0));
    }
    return false;
}

void
FileManager::Load(ResourceData *res, const std::string &name)
{
    try
    {
        FileBuffer *buffer;
        buffer = LoadResource(name);
        res->Load(buffer);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::Load");
        throw;
    }
}

void
FileManager::Save(ResourceData *res, const std::string &name)
{
    try
    {
        FileBuffer *buffer = new FileBuffer(0x20000);
        unsigned int size = res->Save(buffer);
        SaveResource(name, buffer, size);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::Save");
        throw;
    }
}

void
FileManager::ExtractResource(const std::string &name)
{
    try
    {
        FileBuffer *buffer;
        buffer = LoadResource(name);
        SaveResource(name, buffer);
        delete buffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::ExtractResource");
        throw;
    }
}

void
FileManager::ExtractAllResources()
{
    try
    {
        std::string resName;
        ResourceIndexData resIdxData = {0, 0, 0};
        if (resIndex.GetFirst(resName, resIdxData))
        {
            do
            {
                FileBuffer *buffer = new FileBuffer(resIdxData.size);
                resArchive.LoadResource(*buffer, resIdxData.offset);
                SaveResource(resName, buffer);
                delete buffer;
            }
            while (resIndex.GetNext(resName, resIdxData));
        }
    }
    catch (Exception &e)
    {
        e.Print("FileManager::ExtractAllResources");
        throw;
    }
}

void
FileManager::ArchiveAllResources()
{
    try
    {
        std::string resName;
        ResourceIndexData resIdxData = {0, 0, 0};
        FileBuffer *archiveBuffer = new FileBuffer(0x1000000);
        if (resIndex.GetFirst(resName, resIdxData))
        {
            do
            {
                FileBuffer *buffer = new FileBuffer(resIdxData.size);
                resArchive.LoadResource(*buffer, resIdxData.offset);
                archiveBuffer->PutString(resName, RES_FILENAME_LEN);
                archiveBuffer->PutUint32LE(resIdxData.size);
                archiveBuffer->CopyFrom(buffer, resIdxData.size);
                delete buffer;
            }
            while (resIndex.GetNext(resName, resIdxData));
        }
        SaveResource(resIndex.GetResourceFilename(), archiveBuffer, archiveBuffer->GetBytesDone());
        resIndex.Save("krondor.rmf");
        delete archiveBuffer;
    }
    catch (Exception &e)
    {
        e.Print("FileManager::ArchiveAllResources");
        throw;
    }
}
