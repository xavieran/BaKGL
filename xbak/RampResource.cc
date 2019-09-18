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
#include "RampResource.h"

RampResource::RampResource()
: ramp()
{
}

RampResource::~RampResource()
{
    Clear();
}

unsigned int RampResource::GetSize() const
{
    return ramp.size();
}

std::vector<unsigned char>& RampResource::GetRamp(unsigned int rmp)
{
    return ramp[rmp];
}

unsigned char RampResource::GetColor(unsigned int rmp, unsigned int n) const
{
    return ramp[rmp][n];
}

void RampResource::Clear()
{
    ramp.clear();
}

void RampResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        while (buffer->GetBytesLeft() > 0)
        {
            std::vector<unsigned char> colors(256);
            for (unsigned int i = 0; i < 256; i++)
            {
                colors[i] = buffer->GetUint8();
            }
            ramp.push_back(colors);
        }
    }
    catch (Exception &e)
    {
        e.Print("RampResource::Load");
        throw;
    }
}

unsigned int RampResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("RampResource::Save");
        throw;
    }
}
