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

#include "AnimationResource.h"
#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "FontResource.h"
#include "GameApplication.h"
#include "MoviePlayer.h"
#include "ObjectResource.h"
#include "PointerManager.h"
#include "SDL_Toolkit.h"
#include "Text.h"

GameApplication* GameApplication::instance = 0;

GameApplication::GameApplication()
        : done ( false )
        , inputGrabbed ( false )
        , game()
        , state ( 0 )
        , screenSaveCount ( 0 )
{
    MediaToolkit* media = MediaToolkit::GetInstance();
    media->GetVideo()->CreateWindow ( 1 );
    media->GetVideo()->SetMode ( LORES_HICOL );
    media->GetVideo()->Clear();

    PaletteResource pal;
    pal.GetPalette()->Fill();
    pal.GetPalette()->Activate ( 0, WINDOW_COLORS );
    FontResource fnt;
    FileManager::GetInstance()->Load ( &fnt, "GAME.FNT" );
    TextBlock txt ( "xBaK: Betrayal at Krondor  A fan-made remake", 15, 0, 0, 0, false );
    txt.Draw ( 16, 16, 240, 16, fnt.GetFont() );
    media->GetVideo()->Refresh();

    config = new ConfigResource;
    if ( FileManager::GetInstance()->ConfigExists ( "krondor.cfg" ) )
    {
        FileManager::GetInstance()->Load ( config, "krondor.cfg" );
    }
    else
    {
        Preferences *prefs = new Preferences();
        prefs->SetDefaults();
        config->SetPreferences ( prefs );
        FileManager::GetInstance()->Save ( config, "krondor.cfg" );
    }
    game = new GameResource;
    PointerManager::GetInstance()->AddPointer ( "POINTER.BMX" );
    PointerManager::GetInstance()->AddPointer ( "POINTERG.BMX" );

    media->GetClock()->Delay ( 500 );
}

GameApplication::~GameApplication()
{
    if ( config )
    {
        delete config;
    }
    if ( game )
    {
        delete game;
    }
    PointerManager::CleanUp();
    MediaToolkit::CleanUp();
    ObjectResource::CleanUp();
    SoundResource::CleanUp();
    FileManager::CleanUp();
}

GameApplication* GameApplication::GetInstance()
{
    if ( !instance )
    {
        instance = new GameApplication();
    }
    return instance;
}

void GameApplication::CleanUp()
{
    GameStateCast::CleanUp();
    GameStateCamp::CleanUp();
    GameStateChapter::CleanUp();
    GameStateCombat::CleanUp();
    GameStateContents::CleanUp();
    GameStateFullMap::CleanUp();
    GameStateInfo::CleanUp();
    GameStateInitialOptions::CleanUp();
    GameStateIntro::CleanUp();
    GameStateInventory::CleanUp();
    GameStateLoad::CleanUp();
    GameStateMap::CleanUp();
    GameStateOptions::CleanUp();
    GameStatePreferences::CleanUp();
    GameStateSave::CleanUp();
    GameStateWorld::CleanUp();
    if ( instance )
    {
        delete instance;
        instance = 0;
    }
}

Preferences * GameApplication::GetPreferences()
{
    return config->GetPreferences();
}

Game * GameApplication::GetGame()
{
    return game->GetGame();
}

void GameApplication::SetState ( GameState *st )
{
    state = st;
}

void GameApplication::PlayIntro()
{
    try
    {
        AnimationResource anim;
        FileManager::GetInstance()->Load ( &anim, "INTRO.ADS" );
        MovieResource ttm;
        FileManager::GetInstance()->Load ( &ttm, anim.GetAnimationData ( 1 ).resource );
        MoviePlayer moviePlayer;
        moviePlayer.Play ( &ttm.GetMovieChunks(), true );
    }
    catch ( Exception &e )
    {
        e.Print ( "GameApplication::Intro" );
    }
}

void GameApplication::StartNewGame()
{
    FileManager::GetInstance()->Load ( game, "startup.gam" );
    game->GetGame()->GetParty()->ActivateMember ( 0, 0 );
    game->GetGame()->GetParty()->ActivateMember ( 1, 2 );
    game->GetGame()->GetParty()->ActivateMember ( 2, 1 );
    game->GetGame()->GetCamera()->SetPosition ( Vector2D ( 669600, 1064800 ) );
    game->GetGame()->GetCamera()->SetHeading ( SOUTH );
}

void GameApplication::QuitGame()
{
    done = true;
}

void GameApplication::SaveConfig()
{
    FileManager::GetInstance()->Save ( config, "krondor.cfg" );
}

void GameApplication::Run()
{
    try
    {
        state = GameStateIntro::GetInstance();
        MediaToolkit::GetInstance()->AddKeyboardListener ( this );
        MediaToolkit::GetInstance()->AddPointerButtonListener ( this );
        MediaToolkit::GetInstance()->AddTimerListener ( this );
        state->Enter();
        GameState *savedState = state;
        done = false;
        while ( !done )
        {
            if ( state != savedState )
            {
                savedState->Leave();
                state->Enter();
                savedState = state;
            }
            state->Execute();
        }
        savedState->Leave();
        MediaToolkit::GetInstance()->RemoveKeyboardListener ( this );
        MediaToolkit::GetInstance()->RemovePointerButtonListener ( this );
        MediaToolkit::GetInstance()->RemoveTimerListener ( this );
    }
    catch ( Exception &e )
    {
        e.Print ( "GameApplication::Run" );
    }
}

void GameApplication::KeyPressed ( const KeyboardEvent& kbe )
{
    switch ( kbe.GetKey() )
    {
    case KEY_F11:
    {
        screenSaveCount++;
        std::stringstream filenameStream;
        filenameStream << Directories::GetInstance()->GetCapturePath();
        filenameStream << "xbak_" << std::setw ( 3 ) << std::setfill ( '0' ) << screenSaveCount << ".bmp";
        MediaToolkit::GetInstance()->GetVideo()->SaveScreenShot ( filenameStream.str() );
    }
    break;
    case KEY_F12:
        inputGrabbed = !inputGrabbed;
        MediaToolkit::GetInstance()->GetVideo()->GrabInput ( inputGrabbed );
        break;
    default:
        break;
    }
}

void GameApplication::KeyReleased ( const KeyboardEvent& kbe )
{
    switch ( kbe.GetKey() )
    {
    default:
        break;
    }
}

void GameApplication::PointerButtonPressed ( const PointerButtonEvent& pbe )
{
    switch ( pbe.GetButton() )
    {
    case PB_PRIMARY:
    case PB_SECONDARY:
        if ( !inputGrabbed )
        {
            inputGrabbed = true;
            MediaToolkit::GetInstance()->GetVideo()->GrabInput ( true );
        }
        break;
    case PB_TERTIARY:
        if ( inputGrabbed )
        {
            inputGrabbed = false;
            MediaToolkit::GetInstance()->GetVideo()->GrabInput ( false );
        }
        break;
    default:
        break;
    }
}

void GameApplication::PointerButtonReleased ( const PointerButtonEvent& pbe )
{
    switch ( pbe.GetButton() )
    {
    default:
        break;
    }
}

void GameApplication::TimerExpired ( const TimerEvent& te )
{
    if ( te.GetID() == TMR_MOVING )
    {
        state->Move();
    }
    if ( te.GetID() == TMR_TURNING )
    {
        state->Turn();
    }
}
