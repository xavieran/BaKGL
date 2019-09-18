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

#ifndef CHARACTER_BUTTON_WIDGET_H
#define CHARACTER_BUTTON_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ButtonWidget.h"
#include "PlayerCharacter.h"

const unsigned int SELECTED_IMAGE = 7;

class CharacterButtonWidget : public ButtonWidget
{
private:
    PlayerCharacter *character;
    static Image *selectedImage;
public:
    CharacterButtonWidget ( const Rectangle &r, const int a );
    virtual ~CharacterButtonWidget();
    void SetCharacter ( PlayerCharacter *pc );
    void SetImage ( Image *selected );
    void Draw();
    void LeftClick ( const bool toggle, const int x, const int y );
    void RightClick ( const bool toggle, const int x, const int y );
    void Drop ( const int x, const int y );
};

#endif
