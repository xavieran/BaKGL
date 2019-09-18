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

#ifndef TEXT_WIDGET_H
#define TEXT_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Font.h"
#include "Text.h"
#include "Widget.h"

class TextWidget : public Widget
{
private:
    Font *font;
    TextBlock textBlock;
    int textWidth;
    int textHeight;
    HorizontalAlignment horAlign;
    VerticalAlignment vertAlign;
public:
    TextWidget ( const Rectangle &r, Font *f );
    virtual ~TextWidget();
    void SetColor ( const int c );
    void SetText ( const std::string& s );
    void SetShadow ( const int s, const int xoff, const int yoff );
    void SetAlignment ( const HorizontalAlignment ha, const VerticalAlignment va );
    void SetItalic ( const bool it );
    void Draw();
    void Drag ( const int x, const int y );
    void Drop ( const int x, const int y );
};

#endif
