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

#ifndef TAGGED_RESOURCE_H
#define TAGGED_RESOURCE_H

#include <map>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ResourceData.h"

const uint32_t TAG_ADS = 0x3a534441;
const uint32_t TAG_APP = 0x3a505041;
const uint32_t TAG_BIN = 0x3a4e4942;
const uint32_t TAG_BMP = 0x3a504d42;
const uint32_t TAG_DAT = 0x3a544144;
const uint32_t TAG_FNT = 0x3a544e46;
const uint32_t TAG_GID = 0x3a444947;
const uint32_t TAG_INF = 0x3a464e49;
const uint32_t TAG_MAP = 0x3a50414d;
const uint32_t TAG_PAG = 0x3a474150;
const uint32_t TAG_PAL = 0x3a4c4150;
const uint32_t TAG_RES = 0x3a534552;
const uint32_t TAG_SCR = 0x3a524353;
const uint32_t TAG_SND = 0x3a444e53;
const uint32_t TAG_TAG = 0x3a474154;
const uint32_t TAG_TT3 = 0x3a335454;
const uint32_t TAG_TTI = 0x3a495454;
const uint32_t TAG_VER = 0x3a524556;
const uint32_t TAG_VGA = 0x3a414756;

class TaggedResource
            : public ResourceData
{
private:
    std::map<const unsigned int, FileBuffer*> bufferMap;
public:
    TaggedResource();
    virtual ~TaggedResource();
    void ClearTags();
    void Split ( FileBuffer *buffer );
    bool Find ( const unsigned label, FileBuffer* &buffer );
};

#endif

