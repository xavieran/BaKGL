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

#include "TextWidget.h"

TextWidget::TextWidget ( const Rectangle &r, Font *f )
        : Widget ( r )
        , font ( f )
        , textBlock()
        , textWidth ( 0 )
        , textHeight ( 0 )
        , horAlign ( HA_CENTER )
        , vertAlign ( VA_CENTER )
{
}

TextWidget::~TextWidget()
{
}

void TextWidget::SetText ( const std::string &s )
{
    textBlock.SetWords ( s );
    textWidth = 0;
    for ( unsigned int i = 0; i < s.size(); i++ )
    {
        textWidth += font->GetWidth ( s[i] - font->GetFirst() );
    }
    textHeight = font->GetHeight();
}

void TextWidget::SetColor ( const int c )
{
    textBlock.SetColor ( c );
}

void TextWidget::SetShadow ( const int s, const int xoff, const int yoff )
{
    textBlock.SetShadow ( s );
    textBlock.SetShadowXOff ( xoff );
    textBlock.SetShadowYOff ( yoff );
}

void TextWidget::SetAlignment ( const HorizontalAlignment ha, const VerticalAlignment va )
{
    horAlign = ha;
    vertAlign = va;
}

void TextWidget::SetItalic ( const bool it )
{
    textBlock.SetItalic ( it );
}

void TextWidget::Draw()
{
    if ( IsVisible() )
    {
        int xoff = 0;
        int yoff = 0;
        switch ( horAlign )
        {
        case HA_LEFT:
            xoff = 0;
            break;
        case HA_CENTER:
            xoff = ( rect.GetWidth() - textWidth ) / 2;
            break;
        case HA_RIGHT:
            xoff = rect.GetWidth() - textWidth;
            break;
        case HA_FILL:
            xoff = 0;
            break;
        }
        switch ( vertAlign )
        {
        case VA_TOP:
            yoff = 0;
            break;
        case VA_CENTER:
            yoff = ( rect.GetHeight() - textHeight ) / 2;
            break;
        case VA_BOTTOM:
            yoff = rect.GetHeight() - textHeight;
            break;
        }
        textBlock.Draw(rect.GetXPos() + xoff, rect.GetYPos() + yoff, rect.GetWidth(), rect.GetHeight(), font);
    }
}

void TextWidget::Drag ( const int, const int )
{
}

void TextWidget::Drop ( const int, const int )
{
}
