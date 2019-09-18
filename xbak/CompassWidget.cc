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

#include "CompassWidget.h"

const int COMPASS_WIDGET_XPOS   = 144;
const int COMPASS_WIDGET_YPOS   = 121;
const int COMPASS_WIDGET_WIDTH  = 32;
const int COMPASS_WIDGET_HEIGHT = 11;

const Rectangle COMPASS_WIDGET_RECTANGLE = Rectangle(COMPASS_WIDGET_XPOS, COMPASS_WIDGET_YPOS, COMPASS_WIDGET_WIDTH, COMPASS_WIDGET_HEIGHT);

CompassWidget::CompassWidget(Camera *cam, Image *img)
        : Widget(COMPASS_WIDGET_RECTANGLE)
        , camera(cam)
        , compassImage(img)
        , cachedImage(0)
{
    cachedImage = new Image(COMPASS_WIDGET_WIDTH, COMPASS_WIDGET_HEIGHT);
    camera->Attach(this);
    Update();
}

CompassWidget::~CompassWidget()
{
    camera->Detach(this);
    if (cachedImage)
    {
        delete cachedImage;
    }
}

void CompassWidget::Draw()
{
    if (IsVisible())
    {
        if (cachedImage)
        {
            cachedImage->Draw(rect.GetXPos(), rect.GetYPos());
        }
    }
}

void CompassWidget::Update()
{
    if (compassImage)
    {
        int offset = camera->GetHeading();
        int imagewidth = compassImage->GetWidth();
        compassImage->Draw(rect.GetXPos() - offset, rect.GetYPos(), offset, 0, rect.GetWidth(), rect.GetHeight());
        if ((imagewidth - offset) < rect.GetWidth())
        {
            compassImage->Draw(rect.GetXPos() - offset + imagewidth, rect.GetYPos(), 0, 0, rect.GetWidth() - imagewidth + offset, rect.GetHeight());
        }
    }
    if (cachedImage)
    {
        cachedImage->Read(rect.GetXPos(), rect.GetYPos());
    }
}

void CompassWidget::Drag(const int, const int)
{
}

void CompassWidget::Drop(const int, const int)
{
}
