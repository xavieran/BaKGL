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

#include <sstream>

#include "InventoryItem.h"

InventoryItem::InventoryItem(const unsigned int i, const unsigned int v, const unsigned int f)
        : id(i)
        , value(v)
        , flags(f)
{}

InventoryItem::InventoryItem(const InventoryItem &item)
        : id(item.id)
        , value(item.value)
        , flags(item.flags)
{}

InventoryItem::~InventoryItem()
{}

unsigned int
InventoryItem::GetId() const
{
    return id;
}

unsigned int
InventoryItem::GetValue() const
{
    return value;
}

unsigned int
InventoryItem::GetFlags() const
{
    return flags;
}

bool
InventoryItem::IsEquiped() const
{
    return flags & EQUIPED_MASK;
}

void
InventoryItem::Equip(const bool toggle)
{
    if (toggle)
    {
        flags |= EQUIPED_MASK;
    }
    else
    {
        flags &= ~EQUIPED_MASK;
    }
}

InventoryItem &
InventoryItem::operator=(const InventoryItem &item)
{
    id = item.id;
    value = item.value;
    flags = item.flags;
    return *this;
}


SingleInventoryItem::SingleInventoryItem(const unsigned int i)
        : InventoryItem(i, 0, 0)
{}

SingleInventoryItem::~SingleInventoryItem()
{}

const std::string
SingleInventoryItem::ToString() const
{
    return "";
}

bool
SingleInventoryItem::operator==(const SingleInventoryItem &) const
{
    return false;
}

bool
SingleInventoryItem::operator!=(const SingleInventoryItem &) const
{
    return true;
}


MultipleInventoryItem::MultipleInventoryItem(const unsigned int i, const unsigned int n)
        : InventoryItem(i, n, 0)
{}

MultipleInventoryItem::~MultipleInventoryItem()
{}

unsigned int
MultipleInventoryItem::GetAmount() const
{
    return value;
}

void
MultipleInventoryItem::Add(const unsigned int n)
{
    value += n;
}

void
MultipleInventoryItem::Remove(const unsigned int n)
{
    value -= n;
}

const std::string
MultipleInventoryItem::ToString() const
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool
MultipleInventoryItem::operator==(const MultipleInventoryItem &item) const
{
    return (id == item.id);
}

bool
MultipleInventoryItem::operator!=(const MultipleInventoryItem &item) const
{
    return (id != item.id);
}


RepairableInventoryItem::RepairableInventoryItem(const unsigned int i, const unsigned int c)
        : InventoryItem(i, c, REPAIRABLE_MASK)
{}

RepairableInventoryItem::~RepairableInventoryItem()
{}

void
RepairableInventoryItem::Repair(const unsigned int n)
{
    value += n;
}

void
RepairableInventoryItem::Damage(const unsigned int n)
{
    value -= n;
}

const std::string
RepairableInventoryItem::ToString() const
{
    std::stringstream ss;
    ss << value << "%";
    return ss.str();
}

bool
RepairableInventoryItem::operator==(const RepairableInventoryItem &) const
{
    return false;
}

bool
RepairableInventoryItem::operator!=(const RepairableInventoryItem &) const
{
    return true;
}


UsableInventoryItem::UsableInventoryItem(const unsigned int i, const unsigned int u)
        : InventoryItem(i, u, 0)
{}

UsableInventoryItem::~UsableInventoryItem()
{}

void
UsableInventoryItem::Use(const unsigned int n)
{
    value += n;
}

void
UsableInventoryItem::Restore(const unsigned int n)
{
    value -= n;
}

const std::string
UsableInventoryItem::ToString() const
{
    return "";
}

bool
UsableInventoryItem::operator==(const UsableInventoryItem &) const
{
    return false;
}

bool
UsableInventoryItem::operator!=(const UsableInventoryItem &) const
{
    return true;
}
