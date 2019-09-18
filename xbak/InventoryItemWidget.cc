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
#include "InventoryItemWidget.h"
#include "PointerManager.h"
#include "RequestResource.h"

InventoryItemWidget::InventoryItemWidget(const Rectangle &r, const int a)
: ActiveWidget(r, a)
, invItem(0)
, iconImage(0)
, label(0)
, dragged(false)
, selected(false)
{
    SetFocusable(false);
}

InventoryItemWidget::~InventoryItemWidget()
{
    if (label)
    {
        delete label;
    }
}

InventoryItem*
InventoryItemWidget::GetInventoryItem()
{
    return invItem;
}

void
InventoryItemWidget::SetInventoryItem(InventoryItem *item)
{
    if (!item)
    {
        throw NullPointer(__FILE__, __LINE__);
    }
    invItem = item;
}

Image*
InventoryItemWidget::GetImage()
{
    return iconImage;
}

void
InventoryItemWidget::SetImage(Image *icon)
{
    if (!icon)
    {
        throw NullPointer(__FILE__, __LINE__);
    }
    iconImage = icon;
}

void
InventoryItemWidget::SetLabel(const std::string& s, Font *f)
{
    label = new TextWidget(Rectangle(rect.GetXPos(), rect.GetYPos(), rect.GetWidth(), rect.GetHeight()), f);
    label->SetText(s);
    label->SetColor(INFO_TEXT_COLOR);
    label->SetAlignment(HA_RIGHT, VA_BOTTOM);
}

void
InventoryItemWidget::SetDragged(const bool toggle)
{
    dragged = toggle;
}

void
InventoryItemWidget::Draw()
{
    if (IsVisible())
    {
        if (selected)
        {
        }
        if (!dragged)
        {
            if (iconImage)
            {
                iconImage->Draw(rect.GetXPos() + (rect.GetWidth() - iconImage->GetWidth()) / 2,
                                rect.GetYPos() + (rect.GetHeight() - iconImage->GetHeight()) / 2, 0);
            }
            if (label)
            {
                label->Draw();
            }
        }
    }
}

void
InventoryItemWidget::LeftClick(const bool toggle, const int, const int)
{
    if (IsVisible())
    {
        GenerateActionEvent(toggle ? GetAction() : ACT_STOP);
    }
}

void
InventoryItemWidget::RightClick(const bool toggle, const int, const int)
{
    if (IsVisible())
    {
        selected = toggle;
        GenerateActionEvent(toggle ? (GetAction() + RIGHT_CLICK_OFFSET) : ACT_STOP);
    }
}

void
InventoryItemWidget::Drag(const int x, const int y)
{
    dragged = true;
    PointerManager::GetInstance()->SetDraggedWidget(this,
                                                    rect.GetXPos() + (rect.GetWidth() - iconImage->GetWidth()) / 2 - x,
                                                    rect.GetYPos() + (rect.GetHeight() - iconImage->GetHeight()) / 2 - y);
}

void
InventoryItemWidget::Drop(const int, const int)
{
}
