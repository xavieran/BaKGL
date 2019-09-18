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

#ifndef DIALOG_WINDOW_H
#define DIALOG_WINDOW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Palette.h"
#include "PanelWidget.h"

class DialogWindow
{
private:
    PanelWidget *panel;
public:
    DialogWindow ( PanelWidget *panelwidget );
    ~DialogWindow();
    const Rectangle& GetRectangle() const;
    void Draw();
    void FadeIn ( Palette* pal );
    void FadeOut ( Palette* pal );
    void LeftClickWidget ( const bool toggle );
    void RightClickWidget ( const bool toggle );
    void LeftClickWidget ( const bool toggle, const int x, const int y );
    void RightClickWidget ( const bool toggle, const int x, const int y );
    void DragWidget ( const int x, const int y );
    void DropWidget ( const int x, const int y );
    void PointerOverWidget ( const int x, const int y );
    void SelectNextWidget();
    void SelectPreviousWidget();
};

#endif
