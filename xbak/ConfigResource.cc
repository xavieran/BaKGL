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

#include "Exception.h"
#include "ConfigResource.h"

ConfigResource::ConfigResource()
        : prefs(0)
{
}

ConfigResource::~ConfigResource()
{
    if (prefs)
    {
        delete prefs;
    }
}

Preferences * ConfigResource::GetPreferences()
{
    return prefs;
}

void ConfigResource::SetPreferences(Preferences *p)
{
    prefs = p;
}

const unsigned int PREF_SOUND_MASK        = 0x01;
const unsigned int PREF_MUSIC_MASK        = 0x02;
const unsigned int PREF_COMBATMUSIC_MASK  = 0x04;
const unsigned int PREF_INTRODUCTION_MASK = 0x08;
const unsigned int PREF_CD_MUSIC_MASK     = 0x10;

void ConfigResource::Load(FileBuffer *buffer)
{
    if (!prefs)
    {
        prefs = new Preferences();
    }
    try
    {
        prefs->SetStepSize((StepTurnSize)buffer->GetUint8());
        prefs->SetTurnSize((StepTurnSize)buffer->GetUint8());
        prefs->SetDetail((LevelOfDetail)buffer->GetUint8());
        prefs->SetTextSpeed((TextSpeed)buffer->GetUint8());
        unsigned int flags = (unsigned int)buffer->GetUint8();
        prefs->SetSound(flags & PREF_SOUND_MASK);
        prefs->SetMusic(flags & PREF_MUSIC_MASK);
        prefs->SetCombatMusic(flags & PREF_COMBATMUSIC_MASK);
        prefs->SetIntroduction(flags & PREF_INTRODUCTION_MASK);
        prefs->SetCdMusic(flags & PREF_CD_MUSIC_MASK);
    }
    catch (Exception &e)
    {
        e.Print("ConfigResource::Load");
        throw;
    }
}

unsigned int ConfigResource::Save(FileBuffer *buffer)
{
    if (!prefs)
    {
        throw NullPointer(__FILE__, __LINE__, "prefs");
    }
    try
    {
        buffer->Rewind();
        buffer->PutUint8((unsigned int)prefs->GetStepSize());
        buffer->PutUint8((unsigned int)prefs->GetTurnSize());
        buffer->PutUint8((unsigned int)prefs->GetDetail());
        buffer->PutUint8((unsigned int)prefs->GetTextSpeed());
        unsigned int flags = 0;
        if (prefs->GetSound()) flags |= PREF_SOUND_MASK;
        if (prefs->GetMusic()) flags |= PREF_MUSIC_MASK;
        if (prefs->GetCombatMusic()) flags |= PREF_COMBATMUSIC_MASK;
        if (prefs->GetIntroduction()) flags |= PREF_INTRODUCTION_MASK;
        buffer->PutUint8(flags);
        return buffer->GetBytesDone();
    }
    catch (Exception &e)
    {
        e.Print("ConfigResource::Save");
        throw;
    }
}
