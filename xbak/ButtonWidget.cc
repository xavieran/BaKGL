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

#include "ButtonWidget.h"

ButtonWidget::ButtonWidget(const Rectangle &r, const int a)
        : ActiveWidget(r, a)
        , enabled(true)
        , pressed(false)
{}

ButtonWidget::~ButtonWidget()
{}

void
ButtonWidget::SetEnabled(const bool toggle)
{
    enabled = toggle;
}

bool
ButtonWidget::IsEnabled() const
{
    return enabled;
}

void
ButtonWidget::SetPressed(const bool toggle)
{
    pressed = toggle;
}

bool
ButtonWidget::IsPressed() const
{
    return pressed;
}

void
ButtonWidget::Reset()
{
    pressed = false;
}

void
ButtonWidget::Drag(const int, const int)
{}

void
ButtonWidget::Drop(const int, const int)
{}
