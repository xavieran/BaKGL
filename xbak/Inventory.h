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

#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "InventoryItem.h"
#include "Subject.h"

class InventoryData
{
public:
    unsigned int size;
    InventoryItem *item;
    InventoryData ( unsigned int n, InventoryItem *ii ) : size ( n ), item ( ii )
    {};
    bool operator== ( const InventoryData &invdata ) const
    {
        return ( invdata.size == size ) && ( invdata.item == item );
    };
    bool operator< ( const InventoryData &invdata ) const
    {
        return invdata.size < size;
    };
};

class Inventory
            : public Subject
{
private:
    std::list<InventoryData> items;
    std::list<InventoryData>::iterator Find ( SingleInventoryItem* item );
    std::list<InventoryData>::iterator Find ( MultipleInventoryItem* item );
    std::list<InventoryData>::iterator Find ( RepairableInventoryItem* item );
    std::list<InventoryData>::iterator Find ( UsableInventoryItem* item );
public:
    Inventory();
    ~Inventory();
    unsigned int GetSize() const;
    InventoryItem* GetItem ( const unsigned int n ) const;
    void Add ( InventoryItem *item );
    void Remove ( InventoryItem *item );
    void Add ( SingleInventoryItem *item );
    void Remove ( SingleInventoryItem *item );
    void Add ( MultipleInventoryItem *item );
    void Remove ( MultipleInventoryItem *item );
    void Add ( RepairableInventoryItem *item );
    void Remove ( RepairableInventoryItem *item );
    void Add ( UsableInventoryItem *item );
    void Remove ( UsableInventoryItem *item );
};

#endif
