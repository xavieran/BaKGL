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

#include "MediaToolkit.h"
#include "PopUpWidget.h"

PopUpWidget::PopUpWidget ( const Rectangle &r )
        : ContainerWidget ( r )
{
}

PopUpWidget::~PopUpWidget()
{
}

void
PopUpWidget::Draw()
{
    if ( IsVisible() )
    {
        Video *video = MediaToolkit::GetInstance()->GetVideo();
        video->FillRect ( rect.GetXPos() + 1, rect.GetYPos() + 1, rect.GetWidth() - 2, rect.GetHeight() - 2, POPUP_COLOR );
        video->DrawVLine ( rect.GetXPos(), rect.GetYPos(), rect.GetHeight(), SHADOW_COLOR );
        video->DrawHLine ( rect.GetXPos() + 1, rect.GetYPos(), rect.GetWidth() - 1, LIGHT_COLOR );
        video->DrawVLine ( rect.GetXPos() + rect.GetWidth() - 1, rect.GetYPos() + 1, rect.GetHeight() - 2, LIGHT_COLOR );
        video->DrawHLine ( rect.GetXPos() + 1, rect.GetYPos() + rect.GetHeight() - 1, rect.GetWidth() - 1, SHADOW_COLOR );
        video->DrawVLine ( rect.GetXPos() - 1, rect.GetYPos() + 1, rect.GetHeight(), COLOR_BLACK );
        video->DrawHLine ( rect.GetXPos(), rect.GetYPos() + rect.GetHeight(), rect.GetWidth() - 1, COLOR_BLACK );
        DrawChildWidgets();
    }
}
