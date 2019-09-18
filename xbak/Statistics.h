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

#ifndef STATISTICS_H
#define STATISTICS_H

#ifdef HAVE_CONFIG
#include "config.h"
#endif

const unsigned int STAT_HEALTH            =  0;
const unsigned int STAT_STAMINA           =  1;
const unsigned int STAT_SPEED             =  2;
const unsigned int STAT_STRENGTH          =  3;
const unsigned int STAT_DEFENSE           =  4;
const unsigned int STAT_CROSSBOW_ACCURACY =  5;
const unsigned int STAT_MELEE_ACCURACY    =  6;
const unsigned int STAT_CASTING_ACCURACY  =  7;
const unsigned int STAT_ASSESSMENT        =  8;
const unsigned int STAT_ARMORCRAFT        =  9;
const unsigned int STAT_WEAPONCRAFT       = 10;
const unsigned int STAT_BARDING           = 11;
const unsigned int STAT_HAGGLING          = 12;
const unsigned int STAT_LOCKPICK          = 13;
const unsigned int STAT_SCOUTING          = 14;
const unsigned int STAT_STEALTH           = 15;
const unsigned int NUM_STATS              = 16;

const unsigned int STAT_MAXIMUM    = 0;
const unsigned int STAT_CURRENT    = 1;
const unsigned int STAT_ACTUAL     = 2;
const unsigned int STAT_EXPERIENCE = 3;
const unsigned int STAT_MODIFIER   = 4;
const unsigned int NUM_STAT_VALUES = 5;

typedef int StatValues[NUM_STAT_VALUES];

class Statistics
{
private:
    StatValues statMatrix[NUM_STATS];
public:
    Statistics();
    virtual ~Statistics();
    int Get ( const unsigned int stat, const unsigned int type ) const;
    void Set ( const unsigned int stat, const unsigned int type, const unsigned int value );
};

#endif
