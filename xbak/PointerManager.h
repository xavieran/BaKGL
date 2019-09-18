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

#ifndef POINTER_MANAGER_H
#define POINTER_MANAGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "InventoryItemWidget.h"
#include "Pointer.h"

const unsigned int NORMAL_POINTER = 0;
const unsigned int SPECIAL_POINTER = 1;

class PointerManager
            : public PointerButtonEventListener
            , public PointerMotionEventListener
{
private:
    unsigned int currentPointer;
    bool pressed;
    bool dragged;
    InventoryItemWidget *itemWidget;
    std::vector<Pointer *> pointerVec;
    std::list<DragEventListener *> dragListeners;
    static PointerManager *instance;
protected:
    PointerManager();
public:
    ~PointerManager();
    static PointerManager* GetInstance();
    static void CleanUp();
    Pointer* GetCurrentPointer();
    void SetCurrentPointer ( unsigned int n );
    InventoryItemWidget* GetDraggedWidget ( void );
    void SetDraggedWidget ( InventoryItemWidget *widget, const int x, const int y );
    void AddPointer ( const std::string& resname );
    void PointerButtonPressed ( const PointerButtonEvent &pbe );
    void PointerButtonReleased ( const PointerButtonEvent &pbe );
    void PointerMoved ( const PointerMotionEvent &pme );
    void AddDragListener ( DragEventListener *del );
    void RemoveDragListener ( DragEventListener *del );
};

#endif
