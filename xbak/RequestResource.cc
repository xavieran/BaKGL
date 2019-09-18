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
#include "RequestResource.h"

RequestResource::RequestResource()
        : popup(false)
        , rect(0, 0, 0, 0)
        , xoff(0)
        , yoff(0)
        , data()
{}

RequestResource::~RequestResource()
{
    Clear();
}

bool
RequestResource::IsPopup() const
{
    return popup;
}

Rectangle &
RequestResource::GetRectangle()
{
    return rect;
}

int
RequestResource::GetXOff() const
{
    return xoff;
}

int
RequestResource::GetYOff() const
{
    return yoff;
}

unsigned int
RequestResource::GetSize() const
{
    return data.size();
}

RequestData
RequestResource::GetRequestData(const unsigned int n) const
{
    return data[n];
}

void
RequestResource::Clear()
{
    data.clear();
}

void
RequestResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        buffer->Skip(2);
        popup = (buffer->GetSint16LE() != 0);
        buffer->Skip(2);
        rect.SetXPos(buffer->GetSint16LE());
        rect.SetYPos(buffer->GetSint16LE());
        rect.SetWidth(buffer->GetSint16LE());
        rect.SetHeight(buffer->GetSint16LE());
        buffer->Skip(2);
        xoff = buffer->GetSint16LE();
        yoff = buffer->GetSint16LE();
        buffer->Skip(2);
        buffer->Skip(2);
        buffer->Skip(2);
        buffer->Skip(2);
        unsigned int numRecords = buffer->GetUint16LE();
        int *offset = new int[numRecords];
        for (unsigned int i = 0; i < numRecords; i++)
        {
            RequestData reqData;
            reqData.widget = buffer->GetUint16LE();
            reqData.action = buffer->GetSint16LE();
            reqData.visible = (buffer->GetUint8() > 0);
            buffer->Skip(2);
            buffer->Skip(2);
            buffer->Skip(2);
            reqData.xpos = buffer->GetSint16LE();
            reqData.ypos = buffer->GetSint16LE();
            reqData.width = buffer->GetUint16LE();
            reqData.height = buffer->GetUint16LE();
            buffer->Skip(2);
            offset[i] = buffer->GetSint16LE();
            reqData.teleport = buffer->GetSint16LE();
            reqData.image = buffer->GetUint16LE();
            reqData.image = (reqData.image >> 1) + (reqData.image & 1);
            buffer->Skip(2);
            reqData.group = buffer->GetUint16LE();
            buffer->Skip(2);
            data.push_back(reqData);
        }
        buffer->Skip(2);
        unsigned int start = buffer->GetBytesDone();
        for (unsigned int i = 0; i < numRecords; i++)
        {
            if (offset[i] >= 0)
            {
                buffer->Seek(start + offset[i]);
                data[i].label = buffer->GetString();
            }
        }
        delete[] offset;
    }
    catch (Exception &e)
    {
        e.Print("RequestResource::Load");
        throw;
    }
}

unsigned int
RequestResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("RequestResource::Save");
        throw;
    }
}
