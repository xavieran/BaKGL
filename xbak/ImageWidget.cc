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

#include "ImageWidget.h"

ImageWidget::ImageWidget(const Rectangle &r, Image *img)
        : Widget(r)
        , image(0)
{
    image = new Image(img);
}

ImageWidget::~ImageWidget()
{
    delete image;
}

void
ImageWidget::HorizontalFlip()
{
    image->HorizontalFlip();
}

void
ImageWidget::VerticalFlip()
{
    image->VerticalFlip();
}

void
ImageWidget::Draw()
{
    if (IsVisible())
    {
        image->Draw(rect.GetXPos(), rect.GetYPos(), 0, 0, rect.GetWidth(), rect.GetHeight(), 0);
    }
}

void
ImageWidget::Drag(const int, const int)
{}

void
ImageWidget::Drop(const int, const int)
{}
