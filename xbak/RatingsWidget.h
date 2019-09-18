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

#ifndef RATINGS_WIDGET_H
#define RATINGS_WIDGET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PlayerCharacter.h"
#include "TextWidget.h"

class RatingsWidget
            : public Widget
{
private:
    PlayerCharacter *playerCharacter;
    Image *horizontalBorder;
    Image *verticalBorder;
    TextWidget *ratingsLabel;
    TextWidget *conditionLabel;
    TextWidget *healthLabel;
    TextWidget *staminaLabel;
    TextWidget *speedLabel;
    TextWidget *strengthLabel;
    TextWidget *healthOfLabel;
    TextWidget *staminaOfLabel;
    TextWidget *actualHealth;
    TextWidget *actualStamina;
    TextWidget *actualSpeed;
    TextWidget *actualStrength;
    TextWidget *maximumHealth;
    TextWidget *maximumStamina;
    TextWidget *condition;
public:
    RatingsWidget ( const Rectangle &r, PlayerCharacter *pc, Font *f );
    virtual ~RatingsWidget();
    void SetBorders ( Image *hb, Image *vb );
    void Draw();
    void Drag ( const int x, const int y );
    void Drop ( const int x, const int y );
};

#endif
