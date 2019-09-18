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

#include "Directories.h"
#include "Exception.h"
#include "BasicFile.h"

BasicFile::BasicFile()
{}

BasicFile::~BasicFile()
{}

std::string
BasicFile::GetDefaultPath() const
{
    return std::string("");
}

std::string
BasicFile::GetAlternatePath() const
{
    return std::string("");
}

std::string
BasicFile::GetLastResortPath() const
{
    return std::string("");
}

std::string
BasicFile::GetStoragePath() const
{
    return std::string("");
}

void
BasicFile::Open(const std::string &name, const bool writable)
{
    if (writable)
    {
        std::string filename = GetStoragePath() + name;
        ofs.open(filename.c_str(), std::ios::out | std::ios::binary);
        if (ofs.fail())
        {
            throw OpenError(__FILE__, __LINE__, "(" + filename + ")");
        }
    }
    else
    {
        std::string filename = GetDefaultPath() + name;
        ifs.open(filename.c_str(), std::ios::in | std::ios::binary);
        if (ifs.fail())
        {
            ifs.clear();
            filename = GetAlternatePath() + name;
            ifs.open(filename.c_str(), std::ios::in | std::ios::binary);
            if (ifs.fail())
            {
                ifs.clear();
                filename = GetLastResortPath() + name;
                ifs.open(filename.c_str(), std::ios::in | std::ios::binary);
                if (ifs.fail())
                {
                    throw OpenError(__FILE__, __LINE__, "(" + filename + ")");
                }
            }
        }
    }
}

void
BasicFile::Close()
{
    if (ifs.is_open())
    {
        ifs.close();
    }
    if (ofs.is_open())
    {
        ofs.close();
    }
}

void
BasicFile::Seek(const std::streamoff offset)
{
    if (ifs.is_open())
    {
        ifs.seekg(offset, std::ios::beg);
        if (ifs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
    }
    if (ofs.is_open())
    {
        ofs.seekp(offset, std::ios::beg);
        if (ofs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
    }
}

void
BasicFile::SeekEnd(const std::streamoff offset)
{
    if (ifs.is_open())
    {
        ifs.seekg(offset, std::ios::end);
        if (ifs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
    }
    if (ofs.is_open())
    {
        ofs.seekp(offset, std::ios::end);
        if (ofs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
    }
}

std::streamsize
BasicFile::Size()
{
    if (ifs.is_open())
    {
        ifs.seekg(0, std::ios::end);
        if (ifs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
        return ifs.tellg();
    }
    if (ofs.is_open())
    {
        ofs.seekp(0, std::ios::end);
        if (ofs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
        return ofs.tellp();
    }
    return 0;
}

void
BasicFile::Load(FileBuffer &buffer)
{
    try
    {
        buffer.Load(ifs);
    }
    catch (Exception &e)
    {
        e.Print("BasicFile::Load");
        throw;
    }
}

void
BasicFile::Save(FileBuffer &buffer)
{
    try
    {
        buffer.Save(ofs);
    }
    catch (Exception &e)
    {
        e.Print("BasicFile::Save");
        throw;
    }
}

void
BasicFile::Save(FileBuffer &buffer, const unsigned int n)
{
    try
    {
        buffer.Save(ofs, n);
    }
    catch (Exception &e)
    {
        e.Print("BasicFile::Save");
        throw;
    }
}
