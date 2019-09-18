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

#include "SDL_Clock.h"

Uint32
SDL_Clock_TimerHandler(Uint32 ms, void *param)
{
    SDL_UserEvent userEvent;
    SDL_Event event;

    userEvent.type = SDL_USEREVENT;
    userEvent.code = ms;
    userEvent.data1 = param;
    userEvent.data2 = 0;
    event.type = SDL_USEREVENT;
    event.user = userEvent;
    SDL_PushEvent(&event);
    return 0;
}

SDL_Clock::SDL_Clock()
        : timers()
{}

SDL_Clock::~SDL_Clock()
{
    timers.clear();
}

unsigned int
SDL_Clock::GetTicks() const
{
    return SDL_GetTicks();
}

void
SDL_Clock::Delay(int ms)
{
    if (ms > 0)
    {
        SDL_Delay(ms);
    }
}

void
SDL_Clock::StartTimer(unsigned long n, int ms)
{
    SDL_TimerID id = SDL_AddTimer(ms, SDL_Clock_TimerHandler, (void *)n);
    timers.insert(std::pair<const unsigned long, SDL_TimerID>(n, id));
}

void
SDL_Clock::CancelTimer(unsigned long n)
{
    SDL_RemoveTimer(timers[n]);
    timers.erase(n);
}
