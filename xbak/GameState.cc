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

#include <iomanip>
#include <sstream>

#include "Exception.h"
#include "GameApplication.h"
#include "GameState.h"
#include "PreferencesDialogBridge.h"

GameState::GameState()
: gameApp(GameApplication::GetInstance())
, dialogFactory()
, prevState(0)
{
}

GameState::~GameState()
{
}

void
GameState::ChangeState(GameState *state, const bool savePreviousState)
{
    if (savePreviousState)
    {
        state->prevState = this;
    }
    gameApp->SetState(state);
}

void
GameState::Move()
{
}

void
GameState::Turn()
{
}

GameStateCamp* GameStateCamp::instance = 0;

GameStateCamp::GameStateCamp()
{
    dialog = dialogFactory.CreateCampDialog();
}

GameStateCamp::~GameStateCamp()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateCamp*
GameStateCamp::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateCamp();
    }
    return instance;
}

void
GameStateCamp::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateCamp::Enter()
{
    dialog->Enter();
}

void
GameStateCamp::Leave()
{
    dialog->Leave();
}

void
GameStateCamp::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case CAMP_EXIT:
            ChangeState(prevState);
            break;
        case ACT_STOP:
        case CAMP_UNTIL_HEALED:
        case CAMP_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateCast* GameStateCast::instance = 0;

GameStateCast::GameStateCast()
: dialog(0)
{
}

GameStateCast::~GameStateCast()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateCast*
GameStateCast::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateCast();
    }
    return instance;
}

void
GameStateCast::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateCast::Enter()
{
    dialog = dialogFactory.CreateCastDialog();
    dialog->Enter();
}

void
GameStateCast::Leave()
{
    dialog->Leave();
    delete dialog;
    dialog = 0;
}

void
GameStateCast::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case CAST_EXIT:
            ChangeState(GameStateWorld::GetInstance());
            break;
        case CAST_CAMP1:
        case CAST_CAMP2:
            ChangeState(GameStateCamp::GetInstance(), true);
            break;
        case CAST_MEMBER1:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            break;
        case CAST_MEMBER2:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            break;
        case CAST_MEMBER3:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            break;
        case CAST_MEMBER1 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case CAST_MEMBER2 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case CAST_MEMBER3 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case ACT_STOP:
        case ACT_UP:
        case ACT_DOWN:
        case ACT_LEFT:
        case ACT_RIGHT:
        case CAST_TRIANGLE:
        case CAST_SQUARE:
        case CAST_CAST:
        case CAST_BOOKMARK:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateChapter* GameStateChapter::instance = 0;

GameStateChapter::GameStateChapter()
{
}

GameStateChapter::~GameStateChapter()
{
}

GameStateChapter*
GameStateChapter::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateChapter();
    }
    return instance;
}

void
GameStateChapter::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateChapter::Enter()
{
}

void
GameStateChapter::Leave()
{
}

void
GameStateChapter::Execute()
{
    gameApp->GetGame()->GetChapter()->Start();
    ChangeState(GameStateWorld::GetInstance());
}

GameStateCombat* GameStateCombat::instance = 0;

GameStateCombat::GameStateCombat()
{
}

GameStateCombat::~GameStateCombat()
{
}

GameStateCombat*
GameStateCombat::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateCombat();
    }
    return instance;
}

void
GameStateCombat::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateCombat::Enter()
{
}

void
GameStateCombat::Leave()
{
}

void
GameStateCombat::Execute()
{
    ChangeState(GameStateWorld::GetInstance());
}

GameStateContents* GameStateContents::instance = 0;

GameStateContents::GameStateContents()
{
    dialog = dialogFactory.CreateContentsDialog();
}

GameStateContents::~GameStateContents()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateContents*
GameStateContents::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateContents();
    }
    return instance;
}

void
GameStateContents::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateContents::Enter()
{
    dialog->Enter();
}

void
GameStateContents::Leave()
{
    dialog->Leave();
}

void
GameStateContents::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case CONT_EXIT:
            ChangeState(prevState);
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateFullMap* GameStateFullMap::instance = 0;

GameStateFullMap::GameStateFullMap()
{
    dialog = dialogFactory.CreateFullMapDialog();
}

GameStateFullMap::~GameStateFullMap()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateFullMap*
GameStateFullMap::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateFullMap();
    }
    return instance;
}

void
GameStateFullMap::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateFullMap::Enter()
{
    dialog->Enter();
}

void
GameStateFullMap::Leave()
{
    dialog->Leave();
}

void
GameStateFullMap::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case FMAP_EXIT:
            ChangeState(GameStateMap::GetInstance());
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateInfo* GameStateInfo::instance = 0;

GameStateInfo::GameStateInfo()
: dialog(0)
{
}

GameStateInfo::~GameStateInfo()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateInfo*
GameStateInfo::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateInfo();
    }
    return instance;
}

void
GameStateInfo::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateInfo::Enter()
{
    dialog = dialogFactory.CreateInfoDialog();
    dialog->Enter();
}

void
GameStateInfo::Leave()
{
    dialog->Leave();
    delete dialog;
    dialog = 0;
}

void
GameStateInfo::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case INFO_EXIT:
            ChangeState(prevState);
            break;
        case ACT_STOP:
        case ACT_UP:
        case ACT_DOWN:
        case ACT_LEFT:
        case ACT_RIGHT:
        case INFO_SPELLS:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateInitialOptions* GameStateInitialOptions::instance = 0;

GameStateInitialOptions::GameStateInitialOptions()
{
    dialog = dialogFactory.CreateOptionsDialog(true);
}

GameStateInitialOptions::~GameStateInitialOptions()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateInitialOptions*
GameStateInitialOptions::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateInitialOptions();
    }
    return instance;
}

void
GameStateInitialOptions::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateInitialOptions::Enter()
{
    dialog->Enter();
}

void
GameStateInitialOptions::Leave()
{
    dialog->Leave();
}

void
GameStateInitialOptions::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case OPT_QUIT:
            gameApp->QuitGame();
            break;
        case OPT_NEW_GAME:
            gameApp->StartNewGame();
            ChangeState(GameStateChapter::GetInstance());
            break;
        case OPT_CONTENTS:
            ChangeState(GameStateContents::GetInstance(), true);
            break;
        case OPT_PREFERENCES:
            ChangeState(GameStatePreferences::GetInstance(), true);
            break;
        case OPT_RESTORE:
            ChangeState(GameStateLoad::GetInstance(), true);
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateIntro* GameStateIntro::instance = 0;

GameStateIntro::GameStateIntro()
{
}

GameStateIntro::~GameStateIntro()
{
}

GameStateIntro*
GameStateIntro::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateIntro();
    }
    return instance;
}

void
GameStateIntro::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateIntro::Enter()
{
}

void
GameStateIntro::Leave()
{
}

void
GameStateIntro::Execute()
{
    if (gameApp->GetPreferences()->GetIntroduction())
    {
        gameApp->PlayIntro();
    }
    ChangeState(GameStateInitialOptions::GetInstance());
}

GameStateInventory* GameStateInventory::instance = 0;

GameStateInventory::GameStateInventory()
: dialog(0)
{
}

GameStateInventory::~GameStateInventory()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateInventory*
GameStateInventory::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateInventory();
    }
    return instance;
}

void
GameStateInventory::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateInventory::Enter()
{
    dialog = dialogFactory.CreateInventoryDialog();
    dialog->Enter();
}

void
GameStateInventory::Leave()
{
    dialog->Leave();
    delete dialog;
    dialog = 0;
}

void
GameStateInventory::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case INV_EXIT:
            ChangeState(prevState);
            break;
        case INV_MEMBER1:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            break;
        case INV_MEMBER2:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            break;
        case INV_MEMBER3:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            break;
        case INV_MEMBER1 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case INV_MEMBER2 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case INV_MEMBER3 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case ACT_STOP:
        case ACT_UP:
        case ACT_DOWN:
        case ACT_LEFT:
        case ACT_RIGHT:
        case INV_UNKNOWN:
        case INV_MORE_INFO:
            break;
        default:
            if ((action >= INVENTORY_OFFSET) && (action < 2 * INVENTORY_OFFSET))
            {
                // TODO
            }
            else
            {
                throw UnexpectedValue(__FILE__, __LINE__, action);
            }
            break;
    }
}

GameStateLoad* GameStateLoad::instance = 0;

GameStateLoad::GameStateLoad()
{
    dialog = dialogFactory.CreateLoadDialog();
}

GameStateLoad::~GameStateLoad()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateLoad*
GameStateLoad::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateLoad();
    }
    return instance;
}

void
GameStateLoad::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateLoad::Enter()
{
    dialog->Enter();
}

void
GameStateLoad::Leave()
{
    dialog->Leave();
}

void
GameStateLoad::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case LOAD_CANCEL:
            ChangeState(prevState);
            break;
        case LOAD_RESTORE:
            ChangeState(prevState);
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateMap* GameStateMap::instance = 0;

GameStateMap::GameStateMap()
: dialog(0)
{
}

GameStateMap::~GameStateMap()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateMap*
GameStateMap::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateMap();
    }
    return instance;
}

void
GameStateMap::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateMap::Enter()
{
    gameApp->GetGame()->GetParty()->SelectMember(-1);
    dialog = dialogFactory.CreateMapDialog();
    dialog->Enter();
}

void
GameStateMap::Leave()
{
    dialog->Leave();
    delete dialog;
    dialog = 0;
}

void
GameStateMap::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case MAP_MAIN:
            ChangeState(GameStateWorld::GetInstance());
            break;
        case MAP_CAMP:
            ChangeState(GameStateCamp::GetInstance(), true);
            break;
        case MAP_FULLMAP:
            ChangeState(GameStateFullMap::GetInstance());
            break;
        case MAP_MEMBER1:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInventory::GetInstance(), true);
            break;
        case MAP_MEMBER2:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            ChangeState(GameStateInventory::GetInstance(), true);
            break;
        case MAP_MEMBER3:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            ChangeState(GameStateInventory::GetInstance(), true);
            break;
        case MAP_MEMBER1 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case MAP_MEMBER2 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case MAP_MEMBER3 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case ACT_STOP:
        case ACT_UP:
        case MAP_UP:
        case ACT_DOWN:
        case MAP_DOWN:
        case ACT_LEFT:
        case MAP_LEFT:
        case ACT_RIGHT:
        case MAP_RIGHT:
        case MAP_ZOOMIN:
        case MAP_ZOOMOUT:
        case MAP_UNKNOWN:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateOptions* GameStateOptions::instance = 0;

GameStateOptions::GameStateOptions()
{
    dialog = dialogFactory.CreateOptionsDialog(false);
}

GameStateOptions::~GameStateOptions()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateOptions*
GameStateOptions::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateOptions();
    }
    return instance;
}

void
GameStateOptions::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateOptions::Enter()
{
    dialog->Enter();
}

void
GameStateOptions::Leave()
{
    dialog->Leave();
}

void
GameStateOptions::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case OPT_QUIT:
            gameApp->QuitGame();
            break;
        case OPT_NEW_GAME:
            gameApp->StartNewGame();
            ChangeState(GameStateChapter::GetInstance());
            break;
        case OPT_CANCEL:
            ChangeState(GameStateWorld::GetInstance());
            break;
        case OPT_CONTENTS:
            ChangeState(GameStateContents::GetInstance(), true);
            break;
        case OPT_PREFERENCES:
            ChangeState(GameStatePreferences::GetInstance(), true);
            break;
        case OPT_RESTORE:
            ChangeState(GameStateLoad::GetInstance(), true);
            break;
        case OPT_SAVE:
            ChangeState(GameStateSave::GetInstance(), true);
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStatePreferences* GameStatePreferences::instance = 0;

GameStatePreferences::GameStatePreferences()
{
    dialog = dialogFactory.CreatePreferencesDialog();
}

GameStatePreferences::~GameStatePreferences()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStatePreferences*
GameStatePreferences::GetInstance()
{
    if (!instance)
    {
        instance = new GameStatePreferences();
    }
    return instance;
}

void
GameStatePreferences::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
    PreferencesDialogBridge::CleanUp();
}

void
GameStatePreferences::Enter()
{
    PreferencesDialogBridge::GetInstance()->Fetch();
    dialog->Enter();
}

void
GameStatePreferences::Leave()
{
    dialog->Leave();
}

void
GameStatePreferences::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case PREF_CANCEL:
            ChangeState(prevState);
            break;
        case PREF_OK:
            PreferencesDialogBridge::GetInstance()->Commit();
            gameApp->SaveConfig();
            ChangeState(prevState);
            break;
        case PREF_DEFAULTS:
            PreferencesDialogBridge::GetInstance()->SetDefaults();
            break;
        case PREF_STEP_SMALL:
        case PREF_STEP_MEDIUM:
        case PREF_STEP_LARGE:
        case PREF_TURN_SMALL:
        case PREF_TURN_MEDIUM:
        case PREF_TURN_LARGE:
        case PREF_DETAIL_MIN:
        case PREF_DETAIL_LOW:
        case PREF_DETAIL_HIGH:
        case PREF_DETAIL_MAX:
        case PREF_TEXT_WAIT:
        case PREF_TEXT_MEDIUM:
        case PREF_TEXT_FAST:
        case PREF_SOUND:
        case PREF_MUSIC:
        case PREF_COMBAT_MUSIC:
        case PREF_CD_MUSIC:
        case PREF_INTRODUCTION:
            PreferencesDialogBridge::GetInstance()->SetSelectState(action);
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateSave* GameStateSave::instance = 0;

GameStateSave::GameStateSave()
{
    dialog = dialogFactory.CreateSaveDialog();
}

GameStateSave::~GameStateSave()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateSave*
GameStateSave::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateSave();
    }
    return instance;
}

void
GameStateSave::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateSave::Enter()
{
    dialog->Enter();
}

void
GameStateSave::Leave()
{
    dialog->Leave();
}

void
GameStateSave::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case SAVE_CANCEL:
            ChangeState(prevState);
            break;
        case SAVE_SAVE:
            ChangeState(prevState);
            break;
        case SAVE_REMOVE_GAME:
            break;
        case SAVE_REMOVE_DIR:
            break;
        case ACT_STOP:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

GameStateWorld* GameStateWorld::instance = 0;

GameStateWorld::GameStateWorld()
: dialog(0)
, moving(0)
, turning(0)
{
}

GameStateWorld::~GameStateWorld()
{
    if (dialog)
    {
        delete dialog;
    }
}

GameStateWorld*
GameStateWorld::GetInstance()
{
    if (!instance)
    {
        instance = new GameStateWorld();
    }
    return instance;
}

void
GameStateWorld::CleanUp()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void
GameStateWorld::Enter()
{
    gameApp->GetGame()->GetParty()->SelectMember(-1);
    dialog = dialogFactory.CreateWorldDialog();
    dialog->Enter();
}

void
GameStateWorld::Leave()
{
    dialog->Leave();
    delete dialog;
    dialog = 0;
}

void
GameStateWorld::Execute()
{
    int action = dialog->Execute();
    switch (action)
    {
        case ACT_ESCAPE:
        case MAIN_OPTIONS:
            ChangeState(GameStateOptions::GetInstance());
            break;
        case MAIN_CAMP:
            ChangeState(GameStateCamp::GetInstance(), true);
            break;
        case MAIN_CAST:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateCast::GetInstance());
            break;
        case MAIN_MAP:
            ChangeState(GameStateMap::GetInstance());
            break;
        case MAIN_MEMBER1:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInventory::GetInstance(), true);
            break;
        case MAIN_MEMBER2:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            ChangeState(GameStateInventory::GetInstance(), true);
            break;
        case MAIN_MEMBER3:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            ChangeState(GameStateInventory::GetInstance(), true);
            break;
        case MAIN_MEMBER1 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(0);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case MAIN_MEMBER2 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(1);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case MAIN_MEMBER3 + RIGHT_CLICK_OFFSET:
            gameApp->GetGame()->GetParty()->SelectMember(2);
            ChangeState(GameStateInfo::GetInstance(), true);
            break;
        case ACT_STOP:
            moving = 0;
            turning = 0;
            break;
        case ACT_LEFT:
        case MAIN_LEFT:
            turning = TURN_LEFT;
            Turn();
            break;
        case ACT_RIGHT:
        case MAIN_RIGHT:
            turning = TURN_RIGHT;
            Turn();
            break;
        case ACT_UP:
        case MAIN_UP:
            moving = MOVE_FORWARD;
            Move();
            break;
        case ACT_DOWN:
        case MAIN_DOWN:
            moving = MOVE_BACKWARD;
            Move();
            break;
        case MAIN_BOOKMARK:
        case MAIN_UNKNOWN:
            break;
        default:
            throw UnexpectedValue(__FILE__, __LINE__, action);
            break;
    }
}

void
GameStateWorld::Move()
{
    if (moving != 0)
    {
        gameApp->GetGame()->GetCamera()->Move(moving * (1 + (int)gameApp->GetPreferences()->GetStepSize()));
        dialog->Update();
        MediaToolkit::GetInstance()->GetClock()->StartTimer(TMR_MOVING, 100);
    }
}

void
GameStateWorld::Turn()
{
    if (turning != 0)
    {
        gameApp->GetGame()->GetCamera()->Turn(turning * (1 + (int)gameApp->GetPreferences()->GetTurnSize()));
        dialog->Update();
        MediaToolkit::GetInstance()->GetClock()->StartTimer(TMR_TURNING, 100);
    }
}
