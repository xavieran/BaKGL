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

#ifndef WIDGET_H
#define WIDGET_H

#include <list>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "EventListener.h"
#include "Geometry.h"

const int NO_SHADOW            = -1;
const int COLOR_BLACK          = 0;
const int SHADOW_COLOR         = 1;
const int LIGHT_COLOR          = 4;
const int TEXT_COLOR_NORMAL    = 10;
const int TEXT_COLOR_DISABLED  = 11;
const int TEXT_COLOR_PRESSED   = 6;
const int BUTTON_COLOR_NORMAL  = 14;
const int BUTTON_COLOR_PRESSED = 11;
const int POPUP_COLOR          = 14;
const int INFO_TEXT_COLOR      = 159;

const int RIGHT_CLICK_OFFSET   = 256;
const int RELEASE_OFFSET       = 512;
const int INVENTORY_OFFSET     = 1024;

class Widget
{
protected:
    Rectangle rect;
    bool visible;
public:
    Widget ( const Rectangle &r );
    virtual ~Widget();
    const Rectangle& GetRectangle() const;
    void SetPosition ( const int x, const int y );
    void SetVisible ( const bool toggle );
    bool IsVisible() const;
    virtual void Draw() = 0;
    virtual void Drag ( const int x, const int y ) = 0;
    virtual void Drop ( const int x, const int y ) = 0;
};

class ActiveWidget
            : public Widget
{
protected:
    int action;
    bool draggable;
    bool focusable;
    std::list<ActionEventListener *> actionListeners;
public:
    ActiveWidget ( const Rectangle &r, const int a );
    virtual ~ActiveWidget();
    int GetAction() const;
    bool IsDraggable() const;
    void SetDraggable ( const bool toggle );
    bool IsFocusable() const;
    void SetFocusable ( const bool toggle );
    void AddActionListener ( ActionEventListener *ael );
    void RemoveActionListener ( ActionEventListener *ael );
    void GenerateActionEvent ( const int a );
    void GenerateActionEvent ( const int a, const int x, const int y );
    void GenerateActionEvent ( const ActionEvent& ae );
    void Focus();
    virtual void Reset();
    virtual void LeftClick ( const bool toggle, const int x, const int y ) = 0;
    virtual void RightClick ( const bool toggle, const int x, const int y ) = 0;
};

#endif
