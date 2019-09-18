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

#ifndef IMAGE_BUTTON_WIDGET_H
#define IMAGE_BUTTON_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ButtonWidget.h"
#include "Image.h"

class ImageButtonWidget
            : public ButtonWidget
{
private:
    Image *normalImage;
    Image *pressedImage;
public:
    ImageButtonWidget ( const Rectangle &r, const int a );
    virtual ~ImageButtonWidget();
    void SetImage ( Image *normal, Image *press );
    void Draw();
    void LeftClick ( const bool toggle, const int x, const int y );
    void RightClick ( const bool toggle, const int x, const int y );
};

#endif
