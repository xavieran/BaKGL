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

#include <cstring>

#include "Exception.h"
#include "Statistics.h"

Statistics::Statistics()
{
    memset(statMatrix, 0, sizeof(statMatrix));
}

Statistics::~Statistics()
{}

int
Statistics::Get(const unsigned int stat, const unsigned int type) const
{
    if ((stat < NUM_STATS) && (type < NUM_STAT_VALUES))
    {
        return statMatrix[stat][type];
    }
    else
    {
        throw IndexOutOfRange(__FILE__, __LINE__, "statMatrix");
    }
}

void
Statistics::Set(const unsigned int stat, const unsigned int type, const unsigned int value)
{
    if ((stat < NUM_STATS) && (type < NUM_STAT_VALUES))
    {
        if (type != STAT_ACTUAL)
        {
            statMatrix[stat][type] = value;
            statMatrix[stat][STAT_ACTUAL] = statMatrix[stat][STAT_CURRENT] + statMatrix[stat][STAT_MODIFIER];
        }
    }
    else
    {
        throw IndexOutOfRange(__FILE__, __LINE__, "statMatrix");
    }
}

