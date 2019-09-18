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

#ifndef CHAPTER_H
#define CHAPTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "EventListener.h"
#include "Zone.h"

class Chapter
            : public KeyboardEventListener
            , public PointerButtonEventListener
            , public TimerEventListener
{
private:
    int number;
    bool delayed;
    Zone zone;
    void PlayIntro();
    void PlayScene ( const int scene );
    void ReadBook ( const int scene );
    void ShowMap();
public:
    Chapter ( const int n );
    virtual ~Chapter();
    int Get() const;
    Zone& GetZone();
    void Next();
    void Start ( const bool maponly = false );
    void KeyPressed ( const KeyboardEvent &kbe );
    void KeyReleased ( const KeyboardEvent &kbe );
    void PointerButtonPressed ( const PointerButtonEvent &pbe );
    void PointerButtonReleased ( const PointerButtonEvent &pbe );
    void TimerExpired ( const TimerEvent &te );
};

#endif
