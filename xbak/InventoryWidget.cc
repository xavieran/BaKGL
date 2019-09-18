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
#include "InventoryWidget.h"
#include "WidgetFactory.h"

InventoryWidget::InventoryWidget(const Rectangle &r, PlayerCharacter *pc, ImageResource& img, Font *f)
        : ActionEventListener()
        , ContainerWidget(r)
        , Observer()
        , character(pc)
        , images(img)
        , font(f)
        , freeSpaces()
{
    character->Attach(this);
    character->GetInventory()->Attach(this);
    Update();
}

InventoryWidget::~InventoryWidget()
{
    character->GetInventory()->Detach(this);
    character->Detach(this);
}

void
InventoryWidget::ActionPerformed(const ActionEvent &ae)
{
    GenerateActionEvent(ae);
}

void
InventoryWidget::Update()
{
    Clear();
    WidgetFactory wf;
    freeSpaces.push_back(rect);
    for (unsigned int i = 0; i < character->GetInventory()->GetSize(); i++)
    {
        InventoryItem *item = character->GetInventory()->GetItem(i);
        if (!(item->IsEquiped()))
        {
            Image *image = images.GetImage(item->GetId());
            int width;
            int height;
            ObjectInfo objInfo = ObjectResource::GetInstance()->GetObjectInfo(item->GetId());
            switch (objInfo.imageSize)
            {
            case 1:
                width = MAX_INVENTORY_ITEM_WIDGET_WIDTH / 2;
                height = MAX_INVENTORY_ITEM_WIDGET_HEIGHT / 2;
                break;
            case 2:
                width = MAX_INVENTORY_ITEM_WIDGET_WIDTH;
                height = MAX_INVENTORY_ITEM_WIDGET_HEIGHT / 2;
                break;
            case 4:
                width = MAX_INVENTORY_ITEM_WIDGET_WIDTH;
                height = MAX_INVENTORY_ITEM_WIDGET_HEIGHT;
                break;
            default:
                throw UnexpectedValue(__FILE__, __LINE__, objInfo.imageSize);
                break;
            }
            std::list<Rectangle>::iterator it = freeSpaces.begin();
            while (it != freeSpaces.end())
            {
                if ((it->GetWidth() > width) && (it->GetHeight() > height))
                {
                    InventoryItemWidget *invitem = wf.CreateInventoryItem(Rectangle(it->GetXPos() + 1,
                                                   it->GetYPos() + 1,
                                                   width,
                                                   height),
                                                   INVENTORY_OFFSET + i,
                                                   item,
                                                   image,
                                                   item->ToString(),
                                                   font,
                                                   this);
                    AddActiveWidget(invitem);
                    Rectangle origFreeSpace(*it);
                    freeSpaces.erase(it);
                    if ((origFreeSpace.GetWidth() - width) > (MAX_INVENTORY_ITEM_WIDGET_WIDTH / 2))
                    {
                        freeSpaces.push_back(Rectangle(origFreeSpace.GetXPos() + width + 1,
                                                       origFreeSpace.GetYPos(),
                                                       origFreeSpace.GetWidth() - width - 1,
                                                       origFreeSpace.GetHeight()));
                    }
                    if ((origFreeSpace.GetHeight() - height) > (MAX_INVENTORY_ITEM_WIDGET_HEIGHT / 2))
                    {
                        freeSpaces.push_back(Rectangle(origFreeSpace.GetXPos(),
                                                       origFreeSpace.GetYPos() + height + 1,
                                                       origFreeSpace.GetWidth(),
                                                       origFreeSpace.GetHeight() - height - 1));
                    }
                    freeSpaces.sort();
                    it = freeSpaces.end();
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    freeSpaces.clear();
    SetVisible(character->IsSelected());
}
