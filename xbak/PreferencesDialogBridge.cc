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
#include "GameApplication.h"
#include "PreferencesDialogBridge.h"
#include "RequestResource.h"

PreferencesDialogBridge* PreferencesDialogBridge::instance = 0;

PreferencesDialogBridge::PreferencesDialogBridge()
{}

PreferencesDialogBridge::~PreferencesDialogBridge()
{}

PreferencesDialogBridge*
PreferencesDialogBridge::GetInstance()
{
    if (!instance)
    {
        instance = new PreferencesDialogBridge();
    }
    return instance;
}


void
PreferencesDialogBridge::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
PreferencesDialogBridge::Fetch()
{
    prefs.Copy(*(GameApplication::GetInstance()->GetPreferences()));
}

void
PreferencesDialogBridge::Commit()
{
    GameApplication::GetInstance()->GetPreferences()->Copy(prefs);
}

bool
PreferencesDialogBridge::GetSelectState(const unsigned int action)
{
    switch (action)
    {
    case PREF_STEP_SMALL:
        return prefs.GetStepSize() == STS_SMALL;
        break;
    case PREF_STEP_MEDIUM:
        return prefs.GetStepSize() == STS_MEDIUM;
        break;
    case PREF_STEP_LARGE:
        return prefs.GetStepSize() == STS_LARGE;
        break;
    case PREF_TURN_SMALL:
        return prefs.GetTurnSize() == STS_SMALL;
        break;
    case PREF_TURN_MEDIUM:
        return prefs.GetTurnSize() == STS_MEDIUM;
        break;
    case PREF_TURN_LARGE:
        return prefs.GetTurnSize() == STS_LARGE;
        break;
    case PREF_DETAIL_MIN:
        return prefs.GetDetail() == LOD_MIN;
        break;
    case PREF_DETAIL_LOW:
        return prefs.GetDetail() == LOD_LOW;
        break;
    case PREF_DETAIL_HIGH:
        return prefs.GetDetail() == LOD_HIGH;
        break;
    case PREF_DETAIL_MAX:
        return prefs.GetDetail() == LOD_MAX;
        break;
    case PREF_TEXT_WAIT:
        return prefs.GetTextSpeed() == TS_WAIT;
        break;
    case PREF_TEXT_MEDIUM:
        return prefs.GetTextSpeed() == TS_MEDIUM;
        break;
    case PREF_TEXT_FAST:
        return prefs.GetTextSpeed() == TS_FAST;
        break;
    case PREF_SOUND:
        return prefs.GetSound();
        break;
    case PREF_MUSIC:
        return prefs.GetMusic();
        break;
    case PREF_COMBAT_MUSIC:
        return prefs.GetCombatMusic();
        break;
    case PREF_CD_MUSIC:
        return prefs.GetCdMusic();
        break;
    case PREF_INTRODUCTION:
        return prefs.GetIntroduction();
        break;
    default:
        throw UnexpectedValue(__FILE__, __LINE__, action);
        break;
    }
    return false;
}

void
PreferencesDialogBridge::SetSelectState(const unsigned int action)
{
    switch (action)
    {
    case PREF_STEP_SMALL:
        prefs.SetStepSize(STS_SMALL);
        break;
    case PREF_STEP_MEDIUM:
        prefs.SetStepSize(STS_MEDIUM);
        break;
    case PREF_STEP_LARGE:
        prefs.SetStepSize(STS_LARGE);
        break;
    case PREF_TURN_SMALL:
        prefs.SetTurnSize(STS_SMALL);
        break;
    case PREF_TURN_MEDIUM:
        prefs.SetTurnSize(STS_MEDIUM);
        break;
    case PREF_TURN_LARGE:
        prefs.SetTurnSize(STS_LARGE);
        break;
    case PREF_DETAIL_MIN:
        prefs.SetDetail(LOD_MIN);
        break;
    case PREF_DETAIL_LOW:
        prefs.SetDetail(LOD_LOW);
        break;
    case PREF_DETAIL_HIGH:
        prefs.SetDetail(LOD_HIGH);
        break;
    case PREF_DETAIL_MAX:
        prefs.SetDetail(LOD_MAX);
        break;
    case PREF_TEXT_WAIT:
        prefs.SetTextSpeed(TS_WAIT);
        break;
    case PREF_TEXT_MEDIUM:
        prefs.SetTextSpeed(TS_MEDIUM);
        break;
    case PREF_TEXT_FAST:
        prefs.SetTextSpeed(TS_FAST);
        break;
    case PREF_SOUND:
        prefs.SetSound(!prefs.GetSound());
        break;
    case PREF_MUSIC:
        prefs.SetMusic(!prefs.GetMusic());
        break;
    case PREF_COMBAT_MUSIC:
        prefs.SetCombatMusic(!prefs.GetCombatMusic());
        break;
    case PREF_CD_MUSIC:
        prefs.SetCdMusic(!prefs.GetCdMusic());
        break;
    case PREF_INTRODUCTION:
        prefs.SetIntroduction(!prefs.GetIntroduction());
        break;
    default:
        throw UnexpectedValue(__FILE__, __LINE__, action);
        break;
    }
}

void
PreferencesDialogBridge::SetDefaults()
{
    prefs.SetDefaults();
}
