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
#include "FileManager.h"
#include "MoviePlayer.h"
#include "PointerManager.h"
#include "SDL_Toolkit.h"
#include "TestApplication.h"
#include "Text.h"
#include "TextWidget.h"

TestApplication* TestApplication::instance = 0;

TestApplication::TestApplication()
        : pal()
        , fnt()
        , img()
        , scr()
        , ttm()
        , wld()
{
    MediaToolkit *media = MediaToolkit::GetInstance();
    media->GetVideo()->CreateWindow ( 1 );
    media->GetVideo()->SetMode ( LORES_HICOL );
    PointerManager::GetInstance()->AddPointer ( "POINTER.BMX" );
    media->AddKeyboardListener ( this );
    media->AddTimerListener ( this );
}

TestApplication::~TestApplication()
{
    MediaToolkit *media = MediaToolkit::GetInstance();
    media->RemoveKeyboardListener ( this );
    media->RemoveTimerListener ( this );
    PointerManager::CleanUp();
    MediaToolkit::CleanUp();
    SoundResource::CleanUp();
    FileManager::CleanUp();
}

TestApplication* TestApplication::GetInstance()
{
    if ( !instance )
    {
        instance = new TestApplication();
    }
    return instance;
}

void TestApplication::CleanUp()
{
    if ( instance )
    {
        delete instance;
        instance = 0;
    }
}

void TestApplication::ActivatePalette()
{
    try
    {
        pal.GetPalette()->Fill();
        pal.GetPalette()->Activate ( 0, 256 );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::ActivatePalette" );
    }
}

void TestApplication::ActivatePalette ( const std::string& name )
{
    try
    {
        FileManager::GetInstance()->Load ( &pal, name );
        pal.GetPalette()->Activate ( 0, 256 );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::ActivatePalette" );
    }
}

void TestApplication::ShowImage ( const std::string& name )
{
    try
    {
        MediaToolkit *media = MediaToolkit::GetInstance();
        FileManager::GetInstance()->Load ( &img, name );
        for ( unsigned int i = 0; i < img.GetNumImages(); i++ )
        {
            media->GetVideo()->Clear();
            img.GetImage ( i )->Draw ( 0, 0 );
            media->GetVideo()->Refresh();
            media->GetClock()->StartTimer ( TMR_TEST_APP, 5000 );
            media->WaitEventLoop();
            media->GetClock()->CancelTimer ( TMR_TEST_APP );
        }
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::ShowImage" );
    }
}

void TestApplication::ShowTaggedImage ( const std::string& name )
{
    try
    {
        MediaToolkit *media = MediaToolkit::GetInstance();
        FileManager::GetInstance()->Load ( &timg, name );
        for ( unsigned int i = 0; i < timg.GetNumImages(); i++ )
        {
            media->GetVideo()->Clear();
            timg.GetImage ( i )->Draw ( 0, 0 );
            media->GetVideo()->Refresh();
            media->GetClock()->StartTimer ( TMR_TEST_APP, 5000 );
            media->WaitEventLoop();
            media->GetClock()->CancelTimer ( TMR_TEST_APP );
        }
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::ShowImage" );
    }
}

void TestApplication::ShowScreen ( const std::string& name )
{
    try
    {
        MediaToolkit *media = MediaToolkit::GetInstance();
        FileManager::GetInstance()->Load ( &scr, name );
        if ( scr.GetImage()->IsHighResLowCol() )
        {
            media->GetVideo()->SetMode ( HIRES_LOWCOL );
        }
        media->GetVideo()->Clear();
        scr.GetImage()->Draw ( 0, 0 );
        media->GetVideo()->Refresh();
        media->GetClock()->StartTimer ( TMR_TEST_APP, 10000 );
        media->WaitEventLoop();
        media->GetClock()->CancelTimer ( TMR_TEST_APP );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::ShowScreen" );
    }
}

void TestApplication::DrawFont ( const std::string& name )
{
    try
    {
        MediaToolkit *media = MediaToolkit::GetInstance();
        FileManager::GetInstance()->Load ( &fnt, name );
        media->GetVideo()->Clear();
        Text txt;
        Paragraph pg1 ( fnt.GetFont() );
        TextBlock tb1a ( "The quick brown fox jumped over the lazy dog.", 15, NO_SHADOW, 0, 0, false );
        pg1.AddTextBlock ( tb1a );
        TextBlock tb1b ( "The quick brown fox jumped over the lazy dog.", 15, NO_SHADOW, 0, 0, true );
        pg1.AddTextBlock ( tb1b );
        txt.AddParagraph ( pg1 );
        Paragraph pg2 ( fnt.GetFont() );
        pg2.SetIndent( 20 );
        TextBlock tb2 ( "This is just a rather long text to test wrapping lines at the end of the text box. Just some more text to be sure that the text will be wrapped around at the edges.", 15, NO_SHADOW, 0, 0, false );
        pg2.AddTextBlock ( tb2 );
        txt.AddParagraph ( pg2 );
        txt.GenerateLines ( 280, 0 );
        txt.DrawPage ( 10, 10, 280, 180 );
        media->GetVideo()->Refresh();
        media->GetClock()->StartTimer ( TMR_TEST_APP, 5000 );
        media->WaitEventLoop();
        media->GetClock()->CancelTimer ( TMR_TEST_APP );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::DrawFont" );
    }
}

void TestApplication::PlayMovie ( const std::string& name )
{
    try
    {
        FileManager::GetInstance()->Load ( &ttm, name );
        MoviePlayer moviePlayer;
        moviePlayer.Play ( &ttm.GetMovieChunks(), false );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::PlayMovie" );
    }
}

void TestApplication::PlaySound ( const unsigned int index )
{
    try
    {
        MediaToolkit *media = MediaToolkit::GetInstance();
        SoundData data = SoundResource::GetInstance()->GetSoundData ( index );
        unsigned int channel = MediaToolkit::GetInstance()->GetAudio()->PlaySound ( data.sounds[0]->GetSamples() );
        media->GetClock()->StartTimer ( TMR_TEST_APP, ( index < 1000 ? 5000 : 30000 ) );
        media->WaitEventLoop();
        media->GetClock()->CancelTimer ( TMR_TEST_APP );
        media->GetAudio()->StopSound ( channel );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::PlaySound" );
    }
}

void TestApplication::WalkWorld ( const std::string& zone, const std::string& tile )
{
    try
    {
        MediaToolkit *media = MediaToolkit::GetInstance();
        FileManager::GetInstance()->Load ( &img, "Z" + zone + "SLOT0.BMX" );
        FileManager::GetInstance()->Load ( &wld, "T" + zone + tile + ".WLD" );
        img.GetImage ( 0 )->Draw ( 0, 0 );
        Image *image = new Image ( img.GetImage ( 0 )->GetWidth() / 2, img.GetImage ( 0 )->GetHeight() / 2, img.GetImage ( 0 ) );
        image->Draw ( 160, 0 );
        delete image;
        media->GetVideo()->DrawLine ( 10, 10, 200, 150, 1 );
        media->GetVideo()->DrawLine ( 240, 20, 20, 160, 2 );
        media->GetVideo()->DrawLine ( 300, 180, 190, 30, 3 );
        media->GetVideo()->DrawLine ( 40, 170, 250, 40, 4 );
        int x0[3] = {100, 150, 120};
        int y0[3] = {60, 50, 120};
        media->GetVideo()->DrawPolygon ( x0, y0, 3, 5 );
        int x1[6] = {100, 100, 160, 280, 280, 200};
        int y1[6] = {90, 150, 190, 90, 150, 90};
        media->GetVideo()->FillPolygon ( x1, y1, 6, 6 );
        int x2[3] = {-40, 10, -10};
        int y2[3] = {150, 180, 120};
        media->GetVideo()->FillPolygon ( x2, y2, 3, 7 );
        media->GetVideo()->Refresh();
        media->GetClock()->StartTimer ( TMR_TEST_APP, 5000 );
        media->WaitEventLoop();
        media->GetClock()->CancelTimer ( TMR_TEST_APP );
    }
    catch ( Exception &e )
    {
        e.Print ( "TestApplication::PlayMovie" );
    }
}

void TestApplication::KeyPressed ( const KeyboardEvent &kbe )
{
    switch ( kbe.GetKey() )
    {
    case KEY_ESCAPE:
    case KEY_RETURN:
    case KEY_SPACE:
        MediaToolkit::GetInstance()->TerminateEventLoop();
        break;
    case KEY_F11:
        MediaToolkit::GetInstance()->GetVideo()->SaveScreenShot ( "screenshot.bmp" );
        break;
    default:
        break;
    }
}

void TestApplication::KeyReleased ( const KeyboardEvent &kbe )
{
    switch ( kbe.GetKey() )
    {
    default:
        break;
    }
}

void TestApplication::TimerExpired ( const TimerEvent &te )
{
    if ( te.GetID() == TMR_TEST_APP )
    {
        MediaToolkit::GetInstance()->TerminateEventLoop();
    }
}
