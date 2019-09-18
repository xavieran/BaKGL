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
#include "PortraitWidget.h"

PortraitWidget::PortraitWidget(const Rectangle &r, PlayerCharacter *pc)
        : Widget(r)
        , playerCharacter(pc)
        , horizontalBorder(0)
        , verticalBorder(0)
{}

PortraitWidget::~PortraitWidget()
{}

void
PortraitWidget::SetBorders(Image *hb, Image *vb)
{
    if ((!hb) || (!vb))
    {
        throw NullPointer(__FILE__, __LINE__);
    }
    horizontalBorder = hb;
    verticalBorder = vb;
}

void
PortraitWidget::Draw()
{
    if (IsVisible())
    {
        int xoff = 0;
        int yoff = 0;
        if (horizontalBorder && verticalBorder)
        {
            horizontalBorder->Draw(rect.GetXPos() + verticalBorder->GetWidth(), rect.GetYPos(), 0, 0,
                                   rect.GetWidth() - 2 * verticalBorder->GetWidth(), horizontalBorder->GetHeight());
            horizontalBorder->Draw(rect.GetXPos() + verticalBorder->GetWidth(), rect.GetYPos() + rect.GetHeight() - horizontalBorder->GetHeight(), 0, 0,
                                   rect.GetWidth() - 2 * verticalBorder->GetWidth(), horizontalBorder->GetHeight());
            verticalBorder->Draw(rect.GetXPos(), rect.GetYPos(), 0, 0,
                                 verticalBorder->GetWidth(), rect.GetHeight());
            verticalBorder->Draw(rect.GetXPos() + rect.GetWidth() - verticalBorder->GetWidth(), rect.GetYPos(), 0, 0,
                                 verticalBorder->GetWidth(), rect.GetHeight());
            xoff = verticalBorder->GetWidth();
            yoff = horizontalBorder->GetHeight();
        }
        // playerCharacter->GetPortraitImage()->Draw(rect.GetXPos() +xoff, rect.GetYPos() + yoff, 0);
    }
}

void
PortraitWidget::Drag(const int, const int)
{}

void
PortraitWidget::Drop(const int, const int)
{}
