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

#include "EquipmentWidget.h"
#include "Exception.h"
#include "ObjectResource.h"
#include "WidgetFactory.h"

const unsigned int ARMOR_OFFSET    = 60;
const unsigned int CROSSBOW_OFFSET = 30;

EquipmentWidget::EquipmentWidget(const Rectangle &r, PlayerCharacter *pc, ImageResource& img, Image *as, Image *cbs, Font *f)
        : ContainerWidget(r)
        , Observer()
        , character(pc)
        , images(img)
        , armorSlot(as)
        , crossbowSlot(cbs)
        , font(f)
{
    character->Attach(this);
    character->GetInventory()->Attach(this);
    Update();
}

EquipmentWidget::~EquipmentWidget()
{
    character->GetInventory()->Detach(this);
    character->Detach(this);
}

void
EquipmentWidget::Update()
{
    Clear();
    WidgetFactory wf;
    bool armorEquipped = false;
    bool crossbowEquipped = false;
    for (unsigned int i = 0; i < character->GetInventory()->GetSize(); i++)
    {
        InventoryItem *item = character->GetInventory()->GetItem(i);
        if ((item->IsEquiped()))
        {
            Image *image = images.GetImage(item->GetId());
            int width;
            int height;
            int yoffset;
            ObjectInfo objInfo = ObjectResource::GetInstance()->GetObjectInfo(item->GetId());
            switch (objInfo.imageSize)
            {
            case 1:
                width = MAX_EQUIPMENT_ITEM_WIDGET_WIDTH / 2;
                height = MAX_EQUIPMENT_ITEM_WIDGET_HEIGHT / 2;
                break;
            case 2:
                width = MAX_EQUIPMENT_ITEM_WIDGET_WIDTH;
                height = MAX_EQUIPMENT_ITEM_WIDGET_HEIGHT / 2;
                break;
            case 4:
                width = MAX_EQUIPMENT_ITEM_WIDGET_WIDTH;
                height = MAX_EQUIPMENT_ITEM_WIDGET_HEIGHT;
                break;
            default:
                throw UnexpectedValue(__FILE__, __LINE__, objInfo.imageSize);
                break;
            }
            switch (objInfo.type)
            {
            case OT_SWORD:
                yoffset = 0;
                break;
            case OT_CROSSBOW:
                yoffset = CROSSBOW_OFFSET;
                crossbowEquipped = true;
                break;
            case OT_STAFF:
                yoffset = 0;
                break;
            case OT_ARMOR:
                yoffset = ARMOR_OFFSET;
                armorEquipped = true;
                break;
            default:
                throw UnexpectedValue(__FILE__, __LINE__, objInfo.type);
                break;
            }
            EquipmentItemWidget *eqitem = wf.CreateEquipmentItem(Rectangle(rect.GetXPos() + 1, rect.GetYPos() + yoffset + 1, width, height),
                                          objInfo.type, item, image, item->ToString(), font);
            AddWidget(eqitem);
        }
    }
    if (!armorEquipped)
    {
        EquipmentItemWidget *eqitem = wf.CreateEquipmentItem(Rectangle(rect.GetXPos() + 1, rect.GetYPos() + ARMOR_OFFSET + 1,
                                      MAX_EQUIPMENT_ITEM_WIDGET_WIDTH, MAX_EQUIPMENT_ITEM_WIDGET_HEIGHT),
                                      OT_ARMOR, 0, armorSlot, "", font);
        AddWidget(eqitem);
    }
    if ((character->GetCharacterClass() == CLASS_WARRIOR) && (!crossbowEquipped))
    {
        EquipmentItemWidget *eqitem = wf.CreateEquipmentItem(Rectangle(rect.GetXPos() + 1, rect.GetYPos() + CROSSBOW_OFFSET + 1,
                                      MAX_EQUIPMENT_ITEM_WIDGET_WIDTH, MAX_EQUIPMENT_ITEM_WIDGET_HEIGHT / 2),
                                      OT_CROSSBOW, 0, crossbowSlot, "", font);
        AddWidget(eqitem);
    }
    SetVisible(character->IsSelected());
}
