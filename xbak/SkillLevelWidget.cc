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

#include <iomanip>
#include <sstream>

#include "SkillLevelWidget.h"

SkillLevelWidget::SkillLevelWidget(const Rectangle &r, const Side s, Image *sw, Image *bl, Font *f)
        : Widget(r)
        , side(s)
        , sword(sw)
        , blood(bl)
        , skill(0)
        , level(0)
        , value(0)
{
    if (side == LEFT_SIDE)
    {
        skill = new TextWidget(Rectangle(rect.GetXPos() + 34, rect.GetYPos(), 72, 12), f);
        skill->SetAlignment(HA_LEFT, VA_TOP);
        skill->SetColor(TEXT_COLOR_NORMAL);
        skill->SetShadow(SHADOW_COLOR, 1, 1);
        level = new TextWidget(Rectangle(rect.GetXPos() + 107, rect.GetYPos(), 20, 12), f);
        level->SetAlignment(HA_RIGHT, VA_TOP);
        level->SetColor(TEXT_COLOR_NORMAL);
        level->SetShadow(SHADOW_COLOR, 1, 1);
    }
    else
    {
        skill = new TextWidget(Rectangle(rect.GetXPos() + 6, rect.GetYPos(), 72, 12), f);
        skill->SetAlignment(HA_LEFT, VA_TOP);
        skill->SetColor(TEXT_COLOR_NORMAL);
        skill->SetShadow(SHADOW_COLOR, 1, 1);
        level = new TextWidget(Rectangle(rect.GetXPos() + 79, rect.GetYPos(), 20, 12), f);
        level->SetAlignment(HA_RIGHT, VA_TOP);
        level->SetColor(TEXT_COLOR_NORMAL);
        level->SetShadow(SHADOW_COLOR, 1, 1);
    }
}

SkillLevelWidget::~SkillLevelWidget()
{
    if (skill)
    {
        delete skill;
    }
    if (level)
    {
        delete level;
    }
}

void
SkillLevelWidget::SetLevel(const std::string& s, const int v)
{
    skill->SetText(s);
    value = v;
    if (value > 0)
    {
        std::stringstream stream;
        stream << std::setw(2) << std::setfill(' ') << value << '%';
        level->SetText(stream.str());
    }
    else
    {
        level->SetText("N/A");
    }
}

void
SkillLevelWidget::Draw()
{
    if (IsVisible())
    {
        skill->Draw();
        level->Draw();
        sword->Draw(rect.GetXPos(), rect.GetYPos() + 2, 0);
        int w = (int)(blood->GetWidth() * value / 100.0f);
        if (side == LEFT_SIDE)
        {
            blood->Draw(rect.GetXPos() + 31, rect.GetYPos() + 10, blood->GetWidth() - w, 0, w, blood->GetHeight(), 0);
        }
        else
        {
            blood->Draw(rect.GetXPos(), rect.GetYPos() + 10, 0, 0, w, blood->GetHeight(), 0);
        }
    }
}

void
SkillLevelWidget::Drag(const int, const int)
{}

void
SkillLevelWidget::Drop(const int, const int)
{}
