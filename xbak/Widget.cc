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

#include "MediaToolkit.h"
#include "Widget.h"

Widget::Widget(const Rectangle &r)
        : rect(r)
        , visible(true)
{
}

Widget::~Widget()
{
}

const Rectangle& Widget::GetRectangle() const
{
    return rect;
}

void Widget::SetPosition(const int x, const int y)
{
    rect.SetXPos(x);
    rect.SetYPos(y);
}

void Widget::SetVisible(const bool toggle)
{
    visible = toggle;
}

bool Widget::IsVisible() const
{
    return visible;
}


ActiveWidget::ActiveWidget(const Rectangle &r, const int a)
        : Widget(r)
        , action(a)
        , draggable(false)
        , focusable(true)
        , actionListeners()
{
}

ActiveWidget::~ActiveWidget()
{
    actionListeners.clear();
}

int ActiveWidget::GetAction() const
{
    return action;
}

bool ActiveWidget::IsDraggable() const
{
    return draggable;
}

void ActiveWidget::SetDraggable(const bool toggle)
{
    draggable = toggle;
}

bool ActiveWidget::IsFocusable() const
{
    return focusable;
}

void ActiveWidget::SetFocusable(const bool toggle)
{
    focusable = toggle;
}

void ActiveWidget::GenerateActionEvent(const int a)
{
    ActionEvent ae(a, rect.GetXCenter(), rect.GetYCenter());
    for (std::list<ActionEventListener *>::iterator it = actionListeners.begin(); it != actionListeners.end(); ++it)
    {
        (*it)->ActionPerformed(ae);
    }
}

void ActiveWidget::GenerateActionEvent(const int a, const int x, const int y)
{
    ActionEvent ae(a, x, y);
    for (std::list<ActionEventListener *>::iterator it = actionListeners.begin(); it != actionListeners.end(); ++it)
    {
        (*it)->ActionPerformed(ae);
    }
}

void ActiveWidget::GenerateActionEvent(const ActionEvent& ae)
{
    for (std::list<ActionEventListener *>::iterator it = actionListeners.begin(); it != actionListeners.end(); ++it)
    {
        (*it)->ActionPerformed(ae);
    }
}

void ActiveWidget::AddActionListener(ActionEventListener *ael)
{
    actionListeners.push_back(ael);
}

void ActiveWidget::RemoveActionListener(ActionEventListener *ael)
{
    actionListeners.remove(ael);
}

void ActiveWidget::Focus()
{
    if (focusable)
    {
        MediaToolkit::GetInstance()->SetPointerPosition(rect.GetXPos() + rect.GetWidth() / 2, rect.GetYPos() + rect.GetHeight() / 2);
    }
}

void ActiveWidget::Reset()
{
}
