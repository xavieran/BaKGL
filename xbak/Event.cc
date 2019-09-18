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

#include "Event.h"

KeyboardEvent::KeyboardEvent(const Key k)
        : key(k)
{}

KeyboardEvent::~KeyboardEvent()
{}

Key
KeyboardEvent::GetKey() const
{
    return key;
}

PointerButtonEvent::PointerButtonEvent(const PointerButton b, const int x, const int y)
        : button(b)
        , xpos(x)
        , ypos(y)
{}

PointerButtonEvent::~PointerButtonEvent()
{}

PointerButton
PointerButtonEvent::GetButton() const
{
    return button;
}

int
PointerButtonEvent::GetXPos() const
{
    return xpos;
}

int
PointerButtonEvent::GetYPos() const
{
    return ypos;
}

PointerMotionEvent::PointerMotionEvent(const int x, const int y)
        : xpos(x)
        , ypos(y)
{}

PointerMotionEvent::~PointerMotionEvent()
{}

int
PointerMotionEvent::GetXPos() const
{
    return xpos;
}

int
PointerMotionEvent::GetYPos() const
{
    return ypos;
}

TimerEvent::TimerEvent(const unsigned long n)
        : id(n)
{}

TimerEvent::~TimerEvent()
{}

unsigned long
TimerEvent::GetID() const
{
    return id;
}

LoopEvent::LoopEvent(const int t)
        : ticks(t)
{}

LoopEvent::~LoopEvent()
{}

int
LoopEvent::GetTicks() const
{
    return ticks;
}

ActionEvent::ActionEvent(const int a, const int x, const int y)
        : action(a)
        , xpos(x)
        , ypos(y)
{}

ActionEvent::~ActionEvent()
{}

int
ActionEvent::GetAction() const
{
    return action;
}

int
ActionEvent::GetXPos() const
{
    return xpos;
}

int
ActionEvent::GetYPos() const
{
    return ypos;
}

DragEvent::DragEvent(const bool t, const int x, const int y)
        : toggle(t)
        , xpos(x)
        , ypos(y)
{}

DragEvent::~DragEvent()
{}

bool
DragEvent::GetToggle() const
{
    return toggle;
}

int
DragEvent::GetXPos() const
{
    return xpos;
}

int
DragEvent::GetYPos() const
{
    return ypos;
}
