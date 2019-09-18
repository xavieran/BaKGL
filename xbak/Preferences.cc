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

#include "Preferences.h"

Preferences::Preferences()
{
    SetDefaults();
}

Preferences::~Preferences()
{}

void
Preferences::SetDefaults()
{
    stepSize = STS_SMALL;
    turnSize = STS_SMALL;
    textSpeed = TS_WAIT;
    detail = LOD_MAX;
    sound = true;
    music = true;
    combatMusic = true;
    cdMusic = false;
    introduction = true;
}

void
Preferences::Copy(const Preferences &prefs)
{
    stepSize = prefs.stepSize;
    turnSize = prefs.turnSize;
    textSpeed = prefs.textSpeed;
    detail = prefs.detail;
    sound = prefs.sound;
    music = prefs.music;
    combatMusic = prefs.combatMusic;
    introduction = prefs.introduction;
}

StepTurnSize
Preferences::GetStepSize() const
{
    return stepSize;
}

void
Preferences::SetStepSize(const StepTurnSize sz)
{
    stepSize = sz;
}

StepTurnSize
Preferences::GetTurnSize() const
{
    return turnSize;
}

void
Preferences::SetTurnSize(const StepTurnSize sz)
{
    turnSize = sz;
}

TextSpeed
Preferences::GetTextSpeed() const
{
    return textSpeed;
}

void
Preferences::SetTextSpeed(const TextSpeed ts)
{
    textSpeed = ts;
}

LevelOfDetail
Preferences::GetDetail() const
{
    return detail;
}

void
Preferences::SetDetail(const LevelOfDetail lod)
{
    detail = lod;
}

bool
Preferences::GetSound() const
{
    return sound;
}

void
Preferences::SetSound(const bool toggle)
{
    sound = toggle;
}

bool
Preferences::GetMusic() const
{
    return music;
}

void
Preferences::SetMusic(const bool toggle)
{
    music = toggle;
}

bool
Preferences::GetCombatMusic() const
{
    return combatMusic;
}

void
Preferences::SetCombatMusic(const bool toggle)
{
    combatMusic = toggle;
}

bool
Preferences::GetCdMusic() const
{
    return cdMusic;
}

void
Preferences::SetCdMusic(const bool toggle)
{
    cdMusic = toggle;
}

bool
Preferences::GetIntroduction() const
{
    return introduction;
}

void
Preferences::SetIntroduction(const bool toggle)
{
    introduction = toggle;
}
