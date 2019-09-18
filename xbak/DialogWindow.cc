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

#include "DialogWindow.h"
#include "PointerManager.h"

DialogWindow::DialogWindow(PanelWidget *panelwidget)
        : panel(panelwidget)
{
}

DialogWindow::~DialogWindow()
{
    if (panel)
    {
        delete panel;
    }
}

const Rectangle& DialogWindow::GetRectangle() const
{
    return panel->GetRectangle();
}

void DialogWindow::Draw()
{
    panel->Draw();
    PointerManager::GetInstance()->GetCurrentPointer()->Draw();
    MediaToolkit::GetInstance()->GetVideo()->Refresh();
}

void DialogWindow::FadeIn(Palette* pal)
{
    panel->Reset();
    panel->Draw();
    PointerManager::GetInstance()->GetCurrentPointer()->Draw();
    pal->FadeIn(0, WINDOW_COLORS, 64, 5);
}

void DialogWindow::FadeOut(Palette* pal)
{
    panel->Draw();
    PointerManager::GetInstance()->GetCurrentPointer()->Draw();
    pal->FadeOut(0, WINDOW_COLORS, 64, 5);
}

void DialogWindow::LeftClickWidget(const bool toggle)
{
    panel->LeftClick(toggle);
}

void DialogWindow::RightClickWidget(const bool toggle)
{
    panel->RightClick(toggle);
}

void DialogWindow::LeftClickWidget(const bool toggle, const int x, const int y)
{
    panel->LeftClick(toggle, x, y);
}

void DialogWindow::RightClickWidget(const bool toggle, const int x, const int y)
{
    panel->RightClick(toggle, x, y);
}

void DialogWindow::DragWidget(const int x, const int y)
{
    panel->Drag(x, y);
}

void DialogWindow::DropWidget(const int x, const int y)
{
    panel->Drop(x, y);
    PointerManager::GetInstance()->SetDraggedWidget(0, 0, 0);
}

void DialogWindow::PointerOverWidget(const int x, const int y)
{
    panel->PointerOver(x, y);
}

void DialogWindow::SelectNextWidget()
{
    panel->NextWidget();
}

void DialogWindow::SelectPreviousWidget()
{
    panel->PreviousWidget();
}
