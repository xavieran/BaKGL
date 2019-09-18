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

#ifndef MOVIE_PLAYER_H
#define MOVIE_PLAYER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "MovieResource.h"
#include "ImageResource.h"
#include "PaletteResource.h"
#include "SoundResource.h"
#include "ScreenResource.h"

const unsigned int MAX_IMAGE_SLOTS   = 4;
const unsigned int MAX_PALETTE_SLOTS = 4;

class MoviePlayer
            : public KeyboardEventListener
            , public PointerButtonEventListener
            , public TimerEventListener
{
private:
    std::vector<MovieChunk *> *chunkVec;
    ScreenResource* screenSlot;
    SoundResource* soundSlot;
    ImageResource* imageSlot[MAX_IMAGE_SLOTS];
    PaletteResource* paletteSlot[MAX_PALETTE_SLOTS];
    Image *backgroundImage;
    bool backgroundImageDrawn;
    Image *savedImage;
    int xSavedImage;
    int ySavedImage;
    bool savedImageDrawn;
    unsigned int currFrame;
    unsigned int currImage;
    unsigned int currPalette;
    unsigned int currChunk;
    unsigned int currDelay;
    unsigned int currSound;
    std::map<unsigned int, int> soundMap;
    bool paletteActivated;
    bool playing;
    bool looped;
    bool delayed;
    void PlayChunk ( MediaToolkit* media );
public:
    MoviePlayer();
    ~MoviePlayer();
    void Play ( std::vector<MovieChunk *> *movie, const bool repeat );
    void KeyPressed ( const KeyboardEvent& kbe );
    void KeyReleased ( const KeyboardEvent& kbe );
    void PointerButtonPressed ( const PointerButtonEvent& pbe );
    void PointerButtonReleased ( const PointerButtonEvent& pbe );
    void TimerExpired ( const TimerEvent& te );
    void FadeComplete();
};

#endif
