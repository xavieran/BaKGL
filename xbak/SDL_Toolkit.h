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

#ifndef SDL_TOOLKIT_H
#define SDL_TOOLKIT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "MediaToolkit.h"
#include "SDL.h"

class SDL_Toolkit
    : public MediaToolkit
{
    enum JoystickState { JS_CENTER,
                         JS_UP,
                         JS_UP_LEFT,
                         JS_LEFT,
                         JS_DOWN_LEFT,
                         JS_DOWN,
                         JS_DOWN_RIGHT,
                         JS_RIGHT,
                         JS_UP_RIGHT };

  private:
    int xPos;
    int yPos;
    int xMove;
    int yMove;
    enum JoystickState jsState;
    SDL_Joystick *joystick;
    void HandleEvent ( SDL_Event *event );
    void UpdatePointer();
  public:
    SDL_Toolkit();
    ~SDL_Toolkit();
    void PollEvents();
    void PollEventLoop();
    void WaitEvents();
    void WaitEventLoop();
    void ClearEvents();
    void GetPointerPosition ( int *x, int *y );
    void SetPointerPosition ( int x, int y );
};

#endif

