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

#ifndef BADGE_WIDGET_H
#define BADGE_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TextWidget.h"

class BadgeWidget
            : public Widget
{
private:
    TextWidget* label;
public:
    BadgeWidget ( const Rectangle &r );
    virtual ~BadgeWidget();
    void SetLabel ( const std::string& s, Font *f );
    void Draw();
    void Drag ( const int x, const int y );
    void Drop ( const int x, const int y );
};

#endif
