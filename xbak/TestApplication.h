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

#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ImageResource.h"
#include "FontResource.h"
#include "MovieResource.h"
#include "PaletteResource.h"
#include "ScreenResource.h"
#include "SoundResource.h"
#include "TaggedImageResource.h"
#include "TileWorldResource.h"

class TestApplication
            : public KeyboardEventListener
            , public TimerEventListener
{
private:
    PaletteResource pal;
    FontResource fnt;
    ImageResource img;
    ScreenResource scr;
    MovieResource ttm;
    TaggedImageResource timg;
    TileWorldResource wld;
    SoundResource* snd;
    static TestApplication *instance;
protected:
    TestApplication();
public:
    ~TestApplication();
    static TestApplication* GetInstance();
    static void CleanUp();
    void ActivatePalette();
    void ActivatePalette ( const std::string& name );
    void ShowImage ( const std::string& name );
    void ShowTaggedImage ( const std::string& name );
    void ShowScreen ( const std::string& name );
    void DrawFont ( const std::string& name );
    void PlayMovie ( const std::string& name );
    void PlaySound ( const unsigned int index );
    void WalkWorld ( const std::string& zone, const std::string& tile );
    void KeyPressed ( const KeyboardEvent &kbe );
    void KeyReleased ( const KeyboardEvent &kbe );
    void TimerExpired ( const TimerEvent &te );
};

#endif
