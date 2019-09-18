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
#include "ImageButtonWidget.h"
#include "RequestResource.h"

ImageButtonWidget::ImageButtonWidget(const Rectangle &r, const int a)
: ButtonWidget(r, a)
, normalImage(0)
, pressedImage(0)
{
}

ImageButtonWidget::~ImageButtonWidget()
{
}

void
ImageButtonWidget::SetImage(Image *normal, Image *press)
{
    if ((!normal) || (!press))
    {
        throw NullPointer(__FILE__, __LINE__);
    }
    normalImage = normal;
    pressedImage = press;
}

void
ImageButtonWidget::Draw()
{
    if (IsVisible())
    {
        if (IsEnabled())
        {
            if (IsPressed())
            {
                if (pressedImage)
                {
                    pressedImage->Draw(rect.GetXPos(), rect.GetYPos() + 1, 0);
                }
            }
            else
            {
                if (normalImage)
                {
                    normalImage->Draw(rect.GetXPos(), rect.GetYPos() + 1, 0);
                }
            }
        }
    }
}

void
ImageButtonWidget::LeftClick(const bool toggle, const int, const int)
{
    if (IsVisible())
    {
        SetPressed(toggle);
        GenerateActionEvent(toggle ? GetAction() : ACT_STOP);
    }
}

void
ImageButtonWidget::RightClick(const bool toggle, const int, const int)
{
    if (IsVisible())
    {
        if (toggle)
        {
        }
    }
}
