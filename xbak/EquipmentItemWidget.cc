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

#include "EquipmentItemWidget.h"
#include "Exception.h"
#include "PointerManager.h"

EquipmentItemWidget::EquipmentItemWidget(const Rectangle &r, const ObjectType t)
        : Widget(r)
        , type(t)
        , invItem(0)
        , iconImage(0)
        , label(0)
{}

EquipmentItemWidget::~EquipmentItemWidget()
{
    if (label)
    {
        delete label;
    }
}

void
EquipmentItemWidget::SetImage(Image *icon)
{
    if (!icon)
    {
        throw NullPointer(__FILE__, __LINE__);
    }
    iconImage = icon;
}

InventoryItem*
EquipmentItemWidget::GetInventoryItem()
{
    return invItem;
}

void
EquipmentItemWidget::SetInventoryItem(InventoryItem *item)
{
    invItem = item;
}

void
EquipmentItemWidget::SetLabel(const std::string& s, Font *f)
{
    label = new TextWidget(Rectangle(rect.GetXPos(), rect.GetYPos(), rect.GetWidth(), rect.GetHeight()), f);
    label->SetText(s);
    label->SetColor(INFO_TEXT_COLOR);
    label->SetAlignment(HA_RIGHT, VA_BOTTOM);
}

void
EquipmentItemWidget::Draw()
{
    if (IsVisible())
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

void
EquipmentItemWidget::Drag(const int, const int)
{}

void
EquipmentItemWidget::Drop(const int, const int)
{
    InventoryItemWidget *widget = PointerManager::GetInstance()->GetDraggedWidget();
    if (widget)
    {
        InventoryItem *item = widget->GetInventoryItem();
        if (item)
        {
            ObjectInfo objInfo = ObjectResource::GetInstance()->GetObjectInfo(item->GetId());
            if (type == objInfo.type)
            {
                invItem->Equip(false);
                item->Equip(true);
            }
        }
    }
}
