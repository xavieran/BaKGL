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

#ifndef INVENTORY_ITEM_WIDGET_H
#define INVENTORY_ITEM_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Image.h"
#include "TextWidget.h"

const int MAX_INVENTORY_ITEM_WIDGET_WIDTH  = 80;
const int MAX_INVENTORY_ITEM_WIDGET_HEIGHT = 58;

class InventoryItemWidget
            : public ActiveWidget
{
private:
    InventoryItem *invItem;
    Image *iconImage;
    TextWidget *label;
    bool dragged;
    bool selected;
public:
    InventoryItemWidget ( const Rectangle &r, const int a );
    virtual ~InventoryItemWidget();
    InventoryItem* GetInventoryItem();
    void SetInventoryItem ( InventoryItem *item );
    Image* GetImage();
    void SetImage ( Image *icon );
    void SetLabel ( const std::string& s, Font *f );
    void SetDragged( const bool toggle );
    void Draw();
    void LeftClick ( const bool toggle, const int x, const int y );
    void RightClick ( const bool toggle, const int x, const int y );
    void Drag ( const int x, const int y );
    void Drop ( const int x, const int y );
};

#endif
