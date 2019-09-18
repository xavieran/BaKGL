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
#include "Inventory.h"
#include "ObjectResource.h"

Inventory::Inventory()
        : items()
{}

Inventory::~Inventory()
{
    for (std::list<InventoryData>::iterator it = items.begin(); it != items.end(); ++it)
    {
        delete it->item;
    }
    items.clear();
}

unsigned int
Inventory::GetSize() const
{
    return items.size();
}

InventoryItem *
Inventory::GetItem(const unsigned int n) const
{
    std::list<InventoryData>::const_iterator it = items.begin();
    for (unsigned int i = 0; i < n; i++) ++it;
    return it->item;
}

std::list<InventoryData>::iterator
Inventory::Find(SingleInventoryItem* item)
{
    std::list<InventoryData>::iterator it = items.begin();
    while (it != items.end())
    {
        SingleInventoryItem *sii = dynamic_cast<SingleInventoryItem *>(it->item);
        if (sii && (*sii == *item))
        {
            break;
        }
        ++it;
    }
    return it;
}

std::list<InventoryData>::iterator
Inventory::Find(MultipleInventoryItem* item)
{
    std::list<InventoryData>::iterator it = items.begin();
    while (it != items.end())
    {
        MultipleInventoryItem *mii = dynamic_cast<MultipleInventoryItem *>(it->item);
        if (mii && (*mii == *item))
        {
            break;
        }
        ++it;
    }
    return it;
}

std::list<InventoryData>::iterator
Inventory::Find(RepairableInventoryItem* item)
{
    std::list<InventoryData>::iterator it = items.begin();
    while (it != items.end())
    {
        RepairableInventoryItem *rii = dynamic_cast<RepairableInventoryItem *>(it->item);
        if (rii && (*rii == *item))
        {
            break;
        }
        ++it;
    }
    return it;
}

std::list<InventoryData>::iterator
Inventory::Find(UsableInventoryItem* item)
{
    std::list<InventoryData>::iterator it = items.begin();
    while (it != items.end())
    {
        UsableInventoryItem *uii = dynamic_cast<UsableInventoryItem *>(it->item);
        if (uii && (*uii == *item))
        {
            break;
        }
        ++it;
    }
    return it;
}

void
Inventory::Add(InventoryItem* item)
{
    SingleInventoryItem *sii = dynamic_cast<SingleInventoryItem *>(item);
    if (sii)
    {
        Add(sii);
        return;
    }
    MultipleInventoryItem *mii = dynamic_cast<MultipleInventoryItem *>(item);
    if (mii)
    {
        Add(mii);
        return;
    }
    RepairableInventoryItem *rii = dynamic_cast<RepairableInventoryItem *>(item);
    if (rii)
    {
        Add(rii);
        return;
    }
    UsableInventoryItem *uii = dynamic_cast<UsableInventoryItem *>(item);
    if (uii)
    {
        Add(uii);
        return;
    }
}

void
Inventory::Remove(InventoryItem* item)
{
    SingleInventoryItem *sii = dynamic_cast<SingleInventoryItem *>(item);
    if (sii)
    {
        Remove(sii);
        return;
    }
    MultipleInventoryItem *mii = dynamic_cast<MultipleInventoryItem *>(item);
    if (mii)
    {
        Remove(mii);
        return;
    }
    RepairableInventoryItem *rii = dynamic_cast<RepairableInventoryItem *>(item);
    if (rii)
    {
        Remove(rii);
        return;
    }
    UsableInventoryItem *uii = dynamic_cast<UsableInventoryItem *>(item);
    if (uii)
    {
        Remove(uii);
        return;
    }
}

void
Inventory::Add(SingleInventoryItem* item)
{
    items.push_back(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
    items.sort();
    Notify();
}

void
Inventory::Remove(SingleInventoryItem* item)
{
    items.remove(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
    Notify();
}

void
Inventory::Add(MultipleInventoryItem* item)
{
    std::list<InventoryData>::iterator it = Find(item);
    if (it != items.end())
    {
        MultipleInventoryItem *mii = dynamic_cast<MultipleInventoryItem *>(it->item);
        mii->Add(item->GetValue());
        delete item;
    }
    else
    {
        items.push_back(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
        items.sort();
    }
    Notify();
}

void
Inventory::Remove(MultipleInventoryItem* item)
{
    std::list<InventoryData>::iterator it = Find(item);
    if (it != items.end())
    {
        MultipleInventoryItem *mii = dynamic_cast<MultipleInventoryItem *>(it->item);
        if (mii->GetAmount() == item->GetAmount())
        {
            items.remove(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
        }
        else
        {
            mii->Remove(item->GetAmount());
        }
    }
    else
    {
        throw UnexpectedValue(__FILE__, __LINE__, "items.end()");
    }
    Notify();
}

void
Inventory::Add(RepairableInventoryItem* item)
{
    items.push_back(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
    items.sort();
    Notify();
}

void
Inventory::Remove(RepairableInventoryItem* item)
{
    items.remove(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
    Notify();
}

void
Inventory::Add(UsableInventoryItem* item)
{
    items.push_back(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
    items.sort();
    Notify();
}

void
Inventory::Remove(UsableInventoryItem* item)
{
    items.remove(InventoryData(ObjectResource::GetInstance()->GetObjectInfo(item->GetId()).imageSize, item));
    Notify();
}
