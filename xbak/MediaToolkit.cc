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
#include "MediaToolkit.h"
#include "SDL_Toolkit.h"

MediaToolkit* MediaToolkit::instance = 0;

MediaToolkit::MediaToolkit()
        : audio(0)
        , clock(0)
        , video(0)
        , eventLoopRunning(false)
        , keyboardListeners()
        , pointerButtonListeners()
        , pointerMotionListeners()
        , timerListeners()
        , loopListeners()
{}

MediaToolkit::~MediaToolkit()
{
    keyboardListeners.clear();
    pointerButtonListeners.clear();
    pointerMotionListeners.clear();
    timerListeners.clear();
    loopListeners.clear();
}

MediaToolkit*
MediaToolkit::GetInstance()
{
    if (!instance)
    {
        instance = new SDL_Toolkit();
    }
    return instance;
}

void
MediaToolkit::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

Audio*
MediaToolkit::GetAudio() const
{
    return audio;
}

Clock*
MediaToolkit::GetClock() const
{
    return clock;
}

Video*
MediaToolkit::GetVideo() const
{
    return video;
}

void
MediaToolkit::AddKeyboardListener(KeyboardEventListener *kel)
{
    keyboardListeners.push_back(kel);
}

void
MediaToolkit::RemoveKeyboardListener(KeyboardEventListener *kel)
{
    keyboardListeners.remove(kel);
}

void
MediaToolkit::AddPointerButtonListener(PointerButtonEventListener *pbel)
{
    pointerButtonListeners.push_back(pbel);
}

void
MediaToolkit::RemovePointerButtonListener(PointerButtonEventListener *pbel)
{
    pointerButtonListeners.remove(pbel);
}

void
MediaToolkit::AddPointerMotionListener(PointerMotionEventListener *pmel)
{
    pointerMotionListeners.push_back(pmel);
}

void
MediaToolkit::RemovePointerMotionListener(PointerMotionEventListener *pmel)
{
    pointerMotionListeners.remove(pmel);
}

void
MediaToolkit::AddTimerListener(TimerEventListener *tel)
{
    timerListeners.push_back(tel);
}

void
MediaToolkit::RemoveTimerListener(TimerEventListener *tel)
{
    timerListeners.remove(tel);
}

void
MediaToolkit::AddUpdateListener(LoopEventListener *lel)
{
    loopListeners.push_back(lel);
}

void
MediaToolkit::RemoveUpdateListener(LoopEventListener *lel)
{
    loopListeners.remove(lel);
}

void
MediaToolkit::TerminateEventLoop()
{
    eventLoopRunning = false;
}
