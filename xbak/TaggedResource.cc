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
#include "TaggedResource.h"

TaggedResource::TaggedResource()
{}

TaggedResource::~TaggedResource()
{
    ClearTags();
}

void
TaggedResource::ClearTags()
{
    for (std::map<const unsigned int, FileBuffer*>::iterator it = bufferMap.begin(); it != bufferMap.end(); ++it)
    {
        delete it->second;
    }
    bufferMap.clear();
}

void
TaggedResource::Split(FileBuffer *buffer)
{
    while (!buffer->AtEnd())
    {
        unsigned int label = buffer->GetUint32LE();
        switch (label)
        {
        case TAG_ADS:
        case TAG_APP:
        case TAG_BIN:
        case TAG_BMP:
        case TAG_DAT:
        case TAG_FNT:
        case TAG_GID:
        case TAG_INF:
        case TAG_MAP:
        case TAG_PAG:
        case TAG_PAL:
        case TAG_RES:
        case TAG_SCR:
        case TAG_SND:
        case TAG_TAG:
        case TAG_TT3:
        case TAG_TTI:
        case TAG_VER:
        case TAG_VGA:
        {
            unsigned int size = buffer->GetUint32LE();
            std::map<const unsigned int, FileBuffer*>::iterator it = bufferMap.find(label);
            if (it != bufferMap.end())
            {
                delete it->second;
                bufferMap.erase(it);
            }
            if (size & 0x80000000)
            {
                FileBuffer *lblbuf = new FileBuffer(size & 0x7fffffff);
                lblbuf->Fill(buffer);
                bufferMap.insert(std::pair<const unsigned int, FileBuffer*>(label, 0));
                Split(lblbuf);
                delete lblbuf;
            }
            else
            {
                FileBuffer *lblbuf = new FileBuffer(size);
                lblbuf->Fill(buffer);
                bufferMap.insert(std::pair<const unsigned int, FileBuffer*>(label, lblbuf));
            }
        }
        break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, label);
            break;
        }
    }
}

bool
TaggedResource::Find(const unsigned int label, FileBuffer* &buffer)
{
    try
    {
        buffer = bufferMap[label];
    }
    catch (...)
    {
        return false;
    }
    return (buffer != 0);
}
