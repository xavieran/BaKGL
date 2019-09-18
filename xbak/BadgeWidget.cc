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
#include "BadgeWidget.h"

BadgeWidget::BadgeWidget(const Rectangle &r)
        : Widget(r)
        , label(0)
{}

BadgeWidget::~BadgeWidget()
{
    if (label)
    {
        delete label;
    }
}

void
BadgeWidget::SetLabel(const std::string& s, Font *f)
{
    label = new TextWidget(Rectangle(rect.GetXPos() + 2, rect.GetYPos() + 2, rect.GetWidth() - 4, rect.GetHeight() - 4), f);
    label->SetText(s);
    label->SetColor(TEXT_COLOR_NORMAL);
    label->SetShadow(SHADOW_COLOR, 0, 1);
}

void
BadgeWidget::Draw()
{
    if (IsVisible())
    {
        Video *video = MediaToolkit::GetInstance()->GetVideo();
        video->FillRect(rect.GetXPos() + 1, rect.GetYPos() + 1, rect.GetWidth() - 2, rect.GetHeight() - 2, BUTTON_COLOR_NORMAL);
        video->DrawVLine(rect.GetXPos(), rect.GetYPos(), rect.GetHeight(), SHADOW_COLOR);
        video->DrawHLine(rect.GetXPos() + 1, rect.GetYPos(), rect.GetWidth() - 1, LIGHT_COLOR);
        video->DrawVLine(rect.GetXPos() + rect.GetWidth() - 1, rect.GetYPos() + 1, rect.GetHeight() - 2, LIGHT_COLOR);
        video->DrawHLine(rect.GetXPos() + 1, rect.GetYPos() + rect.GetHeight() - 1, rect.GetWidth() - 1, SHADOW_COLOR);
        if (label)
        {
            label->Draw();
        }
    }
}

void
BadgeWidget::Drag(const int, const int)
{}

void
BadgeWidget::Drop(const int, const int)
{}
