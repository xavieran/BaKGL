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

#include "AnimationResource.h"
#include "Exception.h"
#include "ResourceTag.h"

AnimationResource::AnimationResource()
        : TaggedResource()
        , version("")
        , animationMap()
        , script(0)
{}

AnimationResource::~AnimationResource()
{
    Clear();
}

std::string&
AnimationResource::GetVersion()
{
    return version;
}

FileBuffer *
AnimationResource::GetScript() const
{
    return script;
}

AnimationData&
AnimationResource::GetAnimationData(unsigned int id)
{
    return animationMap[id];
}

void
AnimationResource::Clear()
{
    animationMap.clear();
    if (script)
    {
        delete script;
        script = 0;
    }
}

void
AnimationResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        Split(buffer);
        FileBuffer *verbuf;
        FileBuffer *resbuf;
        FileBuffer *scrbuf;
        FileBuffer *tagbuf;
        if (!Find(TAG_VER, verbuf) ||
                !Find(TAG_RES, resbuf) ||
                !Find(TAG_SCR, scrbuf) ||
                !Find(TAG_TAG, tagbuf))
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        version = verbuf->GetString();
        if (scrbuf->GetUint8() != 0x02)
        {
            ClearTags();
            throw DataCorruption(__FILE__, __LINE__);
        }
        script = new FileBuffer(scrbuf->GetUint32LE());
        scrbuf->DecompressLZW(script);
        ResourceTag tags;
        tags.Load(tagbuf);
        unsigned int n = resbuf->GetUint16LE();
        for (unsigned int i = 0; i < n; i++)
        {
            unsigned int id = resbuf->GetUint16LE();
            std::string resource = resbuf->GetString();
            std::string name;
            if (tags.Find(id, name))
            {
                AnimationData data;
                data.name = name;
                data.resource = resource;
                animationMap.insert(std::pair<unsigned int, AnimationData>(id, data));
            }
            else
            {
                throw DataCorruption(__FILE__, __LINE__);
            }
        }
        ClearTags();
    }
    catch (Exception &e)
    {
        e.Print("AnimationResource::Load");
        ClearTags();
        throw;
    }
}

unsigned int
AnimationResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("AnimationResource::Save");
        throw;
    }
}
