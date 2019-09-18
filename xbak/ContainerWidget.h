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

#ifndef CONTAINER_WIDGET_H
#define CONTAINER_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Widget.h"

class ContainerWidget
            : public ActiveWidget
{
private:
    std::list<Widget *> widgets;
    std::list<ActiveWidget *> activeWidgets;
    std::list<ActiveWidget *>::iterator currentActiveWidget;
protected:
    void DrawChildWidgets();
public:
    ContainerWidget ( const Rectangle &r );
    virtual ~ContainerWidget();
    void Draw();
    void Clear();
    void AddWidget ( Widget *w );
    void RemoveWidget ( Widget *w );
    void AddActiveWidget ( ActiveWidget *aw );
    void RemoveActiveWidget ( ActiveWidget *aw );
    void NextWidget();
    void PreviousWidget();
    void LeftClick ( const bool toggle );
    void RightClick ( const bool toggle );
    void LeftClick ( const bool toggle, const int x, const int y );
    void RightClick ( const bool toggle, const int x, const int y );
    void Drag ( const int x, const int y );
    void Drop ( const int x, const int y );
    void PointerOver ( const int x, const int y );
    void Reset();
};

#endif
