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

#ifndef DIALOG_H
#define DIALOG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DialogWindow.h"
#include "Observer.h"

class Dialog
    : public ActionEventListener
    , public PointerButtonEventListener
    , public KeyboardEventListener
    , public DragEventListener
    , public Observer
{
protected:
    DialogWindow *window;
    Palette *palette;
    unsigned int action;
    bool running;
public:
    Dialog ( Palette *pal, DialogWindow *dialogwin );
    virtual ~Dialog();
    void Enter();
    void Leave();
    int Execute();
    void Update();
    void ActionPerformed ( const ActionEvent& ae );
    void PointerButtonPressed ( const PointerButtonEvent& pbe );
    void PointerButtonReleased ( const PointerButtonEvent& pbe );
    void PointerDragged ( const DragEvent &de );
};

class GameDialog
            : public Dialog
{
public:
    GameDialog ( Palette *pal, DialogWindow *dialogwin );
    ~GameDialog();
    void KeyPressed ( const KeyboardEvent& kbe );
    void KeyReleased ( const KeyboardEvent& kbe );
};

class OptionsDialog
            : public Dialog
{
public:
    OptionsDialog ( Palette *pal, DialogWindow *dialogwin );
    ~OptionsDialog();
    void KeyPressed ( const KeyboardEvent& kbe );
    void KeyReleased ( const KeyboardEvent& kbe );
};

#endif
