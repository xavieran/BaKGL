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

#ifndef NULL_AUDIO_H
#define NULL_AUDIO_H

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include "Audio.h"

class Null_Audio
            : public Audio
{
public:
    Null_Audio();
    virtual ~Null_Audio();
    int PlaySound ( FileBuffer *buffer, const int repeat = 0 );
    void StopSound ( const int channel = -1 );
};

#endif
