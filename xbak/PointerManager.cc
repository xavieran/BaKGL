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

#include "ImageResource.h"
#include "MediaToolkit.h"
#include "PointerManager.h"

PointerManager* PointerManager::instance = 0;

PointerManager::PointerManager()
        : currentPointer(0)
        , pressed(false)
        , dragged(false)
        , itemWidget(0)
        , pointerVec()
        , dragListeners()
{
    MediaToolkit::GetInstance()->AddPointerButtonListener(this);
    MediaToolkit::GetInstance()->AddPointerMotionListener(this);
}

PointerManager::~PointerManager()
{
    MediaToolkit::GetInstance()->RemovePointerButtonListener(this);
    MediaToolkit::GetInstance()->RemovePointerMotionListener(this);
    for (unsigned int i = 0; i < pointerVec.size(); i++)
    {
        delete pointerVec[i];
    }
    pointerVec.clear();
    dragListeners.clear();
}

PointerManager*
PointerManager::GetInstance()
{
    if (!instance)
    {
        instance = new PointerManager();
    }
    return instance;
}

void
PointerManager::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

Pointer*
PointerManager::GetCurrentPointer()
{
    return pointerVec[currentPointer];
}

void
PointerManager::SetCurrentPointer(const unsigned int n)
{
    if (n < pointerVec.size())
    {
        currentPointer = n;
    }
    else
    {
        currentPointer = 0;
    }
}

InventoryItemWidget*
PointerManager::GetDraggedWidget()
{
    return itemWidget;
}

void
PointerManager::SetDraggedWidget(InventoryItemWidget *widget, const int x, const int y)
{
    if (itemWidget && (itemWidget != widget))
    {
        itemWidget->SetDragged(false);
    }
    itemWidget = widget;
    if (widget)
    {
        pointerVec[currentPointer]->SetDragImage(widget->GetImage(), x, y);
    }
    else
    {
        pointerVec[currentPointer]->SetDragImage(0, x, y);
    }
}

void
PointerManager::AddPointer(const std::string& resname)
{
    Pointer *mp = new Pointer(resname);
    pointerVec.push_back(mp);
}

void
PointerManager::PointerButtonPressed(const PointerButtonEvent &pbe)
{
    pointerVec[currentPointer]->SetPosition(pbe.GetXPos(), pbe.GetYPos());
    if ((!pressed) && (pbe.GetButton() == PB_PRIMARY))
    {
        pressed = true;
    }
}

void
PointerManager::PointerButtonReleased(const PointerButtonEvent &pbe)
{
    pointerVec[currentPointer]->SetPosition(pbe.GetXPos(), pbe.GetYPos());
    if ((pressed) && (pbe.GetButton() == PB_PRIMARY))
    {
        pressed = false;
        if (dragged)
        {
            dragged = false;
            DragEvent de(dragged, pbe.GetXPos(), pbe.GetYPos());
            for (std::list<DragEventListener *>::iterator it = dragListeners.begin(); it != dragListeners.end(); ++it)
            {
                (*it)->PointerDragged(de);
            }
        }
    }
}

void
PointerManager::PointerMoved(const PointerMotionEvent &pme)
{
    pointerVec[currentPointer]->SetPosition(pme.GetXPos(), pme.GetYPos());
    if ((pressed) && (!dragged))
    {
        dragged = true;
        DragEvent de(dragged, pme.GetXPos(), pme.GetYPos());
        for (std::list<DragEventListener *>::iterator it = dragListeners.begin(); it != dragListeners.end(); ++it)
        {
            (*it)->PointerDragged(de);
        }
    }
}

void
PointerManager::AddDragListener(DragEventListener *del)
{
    dragListeners.push_back(del);
}

void
PointerManager::RemoveDragListener(DragEventListener *del)
{
    dragListeners.remove(del);
}
