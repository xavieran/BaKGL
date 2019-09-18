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

#ifndef LABEL_RESOURCE_H
#define LABEL_RESOURCE_H

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ResourceData.h"

const int LBL_STANDARD = 0;
const int LBL_TITLE    = 3;

struct LabelData
{
    int xpos;
    int ypos;
    int type;
    int color;
    int shadow;
    std::string label;
};

class LabelResource
            : public ResourceData
{
private:
    std::vector<LabelData> data;
public:
    LabelResource();
    virtual ~LabelResource();
    unsigned int GetSize() const;
    LabelData& GetLabelData ( const unsigned int n );
    void Clear();
    void Load ( FileBuffer *buffer );
    unsigned int Save ( FileBuffer *buffer );
};

#endif

