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

#include "GameViewWidget.h"

GameViewWidget::GameViewWidget(const Rectangle &r, Game *g)
        : ContainerWidget(r)
        , game(g)
        , cachedImage(0)
{
    cachedImage = new Image(rect.GetWidth(), rect.GetHeight());
}

GameViewWidget::~GameViewWidget()
{
    if (cachedImage)
    {
        delete cachedImage;
    }
}

void
GameViewWidget::Draw()
{
    if (IsVisible())
    {
        if (cachedImage)
        {
            cachedImage->Draw(rect.GetXPos(), rect.GetYPos());
        }
        DrawChildWidgets();
    }
}

void
GameViewWidget::Update()
{
    Redraw();
    if (cachedImage)
    {
        cachedImage->Read(rect.GetXPos(), rect.GetYPos());
    }
}
