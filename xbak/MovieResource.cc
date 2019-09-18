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

#include <algorithm>
#include <cctype>

#include "Exception.h"
#include "ResourceTag.h"
#include "MovieResource.h"

MovieResource::MovieResource()
        : TaggedResource()
        , version("")
        , pages(0)
        , movieChunks()
{}

MovieResource::~MovieResource()
{
    Clear();
}

std::string&
MovieResource::GetVersion()
{
    return version;
}

unsigned int
MovieResource::GetPages() const
{
    return pages;
}

std::vector<MovieChunk *> &
MovieResource::GetMovieChunks()
{
    return movieChunks;
}

void
MovieResource::Clear()
{
    for (unsigned int i = 0; i < movieChunks.size(); i++)
    {
        movieChunks[i]->data.clear();
        delete movieChunks[i];
    }
    movieChunks.clear();
}

void
MovieResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        Split(buffer);
        FileBuffer *verbuf;
        FileBuffer *pagbuf;
        FileBuffer *tt3buf;
        FileBuffer *tagbuf;
        if (!Find(TAG_VER, verbuf) ||
                !Find(TAG_PAG, pagbuf) ||
                !Find(TAG_TT3, tt3buf) ||
                !Find(TAG_TAG, tagbuf))
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        version = verbuf->GetString();
        pages = pagbuf->GetUint16LE();
        tt3buf->Skip(1);
        FileBuffer *tmpbuf = new FileBuffer(tt3buf->GetUint32LE());
        tt3buf->DecompressRLE(tmpbuf);
        ResourceTag tags;
        tags.Load(tagbuf);
        while (!tmpbuf->AtEnd())
        {
            MovieChunk *mc = new MovieChunk;
            unsigned int code = tmpbuf->GetUint16LE();
            unsigned int size = code & 0x000f;
            code &= 0xfff0;
            mc->code = code;
            if ((code == 0x1110) && (size == 1))
            {
                unsigned int id = tmpbuf->GetUint16LE();
                mc->data.push_back(id);
                std::string name;
                if (tags.Find(id, name))
                {
                    mc->name = name;
                }
            }
            else if (size == 15)
            {
                mc->name = tmpbuf->GetString();
                transform(mc->name.begin(), mc->name.end(), mc->name.begin(), toupper);
                if (tmpbuf->GetBytesLeft() & 1)
                {
                    tmpbuf->Skip(1);
                }
            }
            else
            {
                for (unsigned int i = 0; i < size; i++)
                {
                    mc->data.push_back(tmpbuf->GetSint16LE());
                }
            }
            movieChunks.push_back(mc);
        }
        delete tmpbuf;
        ClearTags();
    }
    catch (Exception &e)
    {
        e.Print("MovieResource::Load");
        ClearTags();
        throw;
    }
}

unsigned int
MovieResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("MovieResource::Save");
        throw;
    }
}
