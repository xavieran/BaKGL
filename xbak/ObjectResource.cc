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
#include "FileManager.h"
#include "ObjectResource.h"

ObjectResource* ObjectResource::instance = 0;

ObjectResource::ObjectResource()
        : data()
{}

ObjectResource::~ObjectResource()
{
    Clear();
}

ObjectResource*
ObjectResource::GetInstance()
{
    if (!instance)
    {
        instance = new ObjectResource();
        FileManager::GetInstance()->Load(instance, "OBJINFO.DAT");
    }
    return instance;
}

void ObjectResource::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

unsigned int
ObjectResource::GetSize() const
{
    return data.size();
}

ObjectInfo&
ObjectResource::GetObjectInfo(unsigned int n)
{
    return data[n];
}

void
ObjectResource::Clear()
{
    data.clear();
}

void
ObjectResource::Load(FileBuffer *buffer)
{
    try
    {
        Clear();
        for (unsigned int i = 0; i < 138; i++)
        {
            ObjectInfo obj;
            obj.name = buffer->GetString(30);
            buffer->Skip(2);
            obj.flags = buffer->GetUint16LE();
            buffer->Skip(2);
            obj.level = buffer->GetSint16LE();
            obj.value = buffer->GetSint16LE();
            obj.strengthSwing = buffer->GetSint16LE();
            obj.strengthThrust = buffer->GetSint16LE();
            obj.accuracySwing = buffer->GetSint16LE();
            obj.accuracyThrust = buffer->GetSint16LE();
            buffer->Skip(2);
            obj.imageSize = buffer->GetUint16LE();
            buffer->Skip(4);
            obj.race = (Race)(buffer->GetUint16LE());
            buffer->Skip(2);
            obj.type = (ObjectType)(buffer->GetUint16LE());
            obj.effectMask = buffer->GetUint16LE();
            obj.effect = buffer->GetSint16LE();
            buffer->Skip(4);
            obj.modifierMask = buffer->GetUint16LE();
            obj.modifier = buffer->GetSint16LE();
            buffer->Skip(6);
            data.push_back(obj);
        }
    }
    catch (Exception &e)
    {
        e.Print("ObjectResource::Load");
        throw;
    }
}

unsigned int
ObjectResource::Save(FileBuffer *buffer)
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch (Exception &e)
    {
        e.Print("ObjectResource::Save");
        throw;
    }
}
