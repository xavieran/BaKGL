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

#include <sstream>

#include "AnimationResource.h"
#include "BookResource.h"
#include "Chapter.h"
#include "Exception.h"
#include "FontResource.h"
#include "FileManager.h"
#include "MoviePlayer.h"
#include "PopUpWidget.h"
#include "TaggedImageResource.h"
#include "TextWidget.h"

Chapter::Chapter ( const int n )
        : number ( n )
        , delayed ( false )
        , zone()
{
    MediaToolkit::GetInstance()->AddKeyboardListener ( this );
    MediaToolkit::GetInstance()->AddPointerButtonListener ( this );
    MediaToolkit::GetInstance()->AddTimerListener ( this );
    zone.Load ( number );
}

Chapter::~Chapter()
{
    MediaToolkit::GetInstance()->RemoveTimerListener ( this );
    MediaToolkit::GetInstance()->RemovePointerButtonListener ( this );
    MediaToolkit::GetInstance()->RemoveKeyboardListener ( this );
}

void Chapter::PlayIntro()
{
    try
    {
        AnimationResource anim;
        std::stringstream filenameStream;
        filenameStream << "CHAPTER" << number << ".ADS";
        FileManager::GetInstance()->Load ( &anim, filenameStream.str() );
        MovieResource ttm;
        FileManager::GetInstance()->Load ( &ttm, anim.GetAnimationData ( 1 ).resource );
        MoviePlayer moviePlayer;
        moviePlayer.Play ( &ttm.GetMovieChunks(), false );
        MediaToolkit::GetInstance()->GetVideo()->Clear();
        MediaToolkit::GetInstance()->GetVideo()->Refresh();
    }
    catch ( Exception &e )
    {
        e.Print ( "Chapter::PlayIntro" );
    }
}

void Chapter::PlayScene ( const int scene )
{
    try
    {
        ScreenResource scr;
        FileManager::GetInstance()->Load ( &scr, "CFRAME.SCX" );
        scr.GetImage()->Draw ( 0, 0 );
        AnimationResource anim;
        std::stringstream filenameStream;
        filenameStream << "C" << number << scene << ".ADS";
        FileManager::GetInstance()->Load ( &anim, filenameStream.str() );
        MovieResource ttm;
        FileManager::GetInstance()->Load ( &ttm, anim.GetAnimationData ( 1 ).resource );
        MoviePlayer moviePlayer;
        moviePlayer.Play ( &ttm.GetMovieChunks(), false );
    }
    catch ( Exception &e )
    {
        e.Print ( "Chapter::PlayIntro" );
    }
}

void Chapter::ReadBook ( const int scene )
{
    const std::string ROMAN_NUMBER[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII", "XIII", "XIV", "XV"};

    try
    {
        PaletteResource pal;
        FileManager::GetInstance()->Load ( &pal, "BOOK.PAL" );
        ScreenResource scr;
        FileManager::GetInstance()->Load ( &scr, "BOOK.SCX" );
        TaggedImageResource img;
        FileManager::GetInstance()->Load ( &img, "BOOK.BMX" );
        FontResource fnt;
        FileManager::GetInstance()->Load ( &fnt, "BOOK.FNT" );
        BookResource bok;
        std::stringstream filenameStream;
        filenameStream << "C" << number << scene << ".BOK";
        FileManager::GetInstance()->Load ( &bok, filenameStream.str() );
        Video *video = MediaToolkit::GetInstance()->GetVideo();
        video->SetMode ( HIRES_LOWCOL );

        Text txt;
        for ( unsigned int i = 0; i < bok.GetSize(); i++ )
        {
            PageData pd = bok.GetPage ( i );
            Image page ( scr.GetImage() );
            if ( pd.number & 1 )
            {
                page.VerticalFlip();
            }
            page.Draw ( 0, 0 );

            for ( unsigned int j = 0; j < pd.decorations.size(); j++ )
            {
                if ( pd.decorations[j].id < img.GetNumImages() )
                {
                    Image deco ( img.GetImage ( pd.decorations[j].id ) );
                    if ( pd.decorations[j].flag & DECO_HORIZONTAL_FLIP )
                    {
                        deco.HorizontalFlip();
                    }
                    if ( pd.decorations[j].flag & DECO_VERTICAL_FLIP )
                    {
                        deco.VerticalFlip();
                    }
                    deco.Draw ( pd.decorations[j].xpos, pd.decorations[j].ypos, 0 );
                }
            }

            int xoff = 0;
            for ( unsigned int j = 0; j < pd.firstLetters.size(); j++ )
            {
                if ( pd.firstLetters[j].id < img.GetNumImages() )
                {
                    Image* letter = img.GetImage ( pd.firstLetters[j].id );
                    letter->Draw ( pd.firstLetters[j].xpos, pd.firstLetters[j].ypos, 0 );
                    xoff = letter->GetWidth() - 49;
                }
            }

            Paragraph paragraph ( fnt.GetFont() );
            paragraph.SetAlignment ( HA_FILL, VA_TOP );
            paragraph.SetIndent ( 30 );
            bool paragraphAdded = false;
            for ( unsigned int j = 0; j < pd.textBlocks.size() ; j++ )
            {
                if ( pd.textBlocks[j].txt.size() > 0 )
                {
                    TextBlock tb ( pd.textBlocks[j].txt, 0, NO_SHADOW, 0, 0, pd.textBlocks[j].italic );
                    if ( pd.textBlocks[j].paragraph )
                    {
                        paragraph.AddTextBlock ( tb );
                    }
                }
                else
                {
                    if ( paragraph.GetSize() > 0 )
                    {
                        txt.AddParagraph ( paragraph );
                        paragraphAdded = true;
                        paragraph.Clear();
                    }
                }
            }
            if ( paragraph.GetSize() > 0 )
            {
                txt.AddParagraph ( paragraph );
                paragraphAdded = true;
            }
            if ( paragraphAdded )
            {
                txt.GenerateLines ( pd.width - 20, xoff );
            }
            txt.DrawPage ( pd.xpos + 10, pd.ypos + 4, pd.width - 20, pd.height - 4 );

            TextWidget pageNumberWidget ( Rectangle ( pd.xpos + 2, pd.ypos + pd.height + 11, pd.width - 4, fnt.GetFont()->GetHeight() ), fnt.GetFont() );
            pageNumberWidget.SetAlignment ( ( pd.number & 1 ) ? HA_RIGHT : HA_LEFT, VA_CENTER );
            pageNumberWidget.SetText ( ROMAN_NUMBER[pd.number] );
            pageNumberWidget.Draw();

            pal.GetPalette()->FadeIn ( 0, WINDOW_COLORS, 64, 5 );
            MediaToolkit::GetInstance()->GetClock()->StartTimer ( TMR_CHAPTER, 10000 );
            delayed = true;
            while ( delayed )
            {
                MediaToolkit::GetInstance()->WaitEvents();
            }
            pal.GetPalette()->FadeOut ( 0, WINDOW_COLORS, 64, 5 );
        }
        video->SetMode ( LORES_HICOL );
    }
    catch ( Exception &e )
    {
        e.Print ( "Chapter::ReadBook" );
    }
}

void Chapter::ShowMap()
{
    try
    {
        PaletteResource pal;
        FileManager::GetInstance()->Load ( &pal, "FULLMAP.PAL" );
        ScreenResource scr;
        FileManager::GetInstance()->Load ( &scr, "FULLMAP.SCX" );
        scr.GetImage()->Draw ( 0, 0 );
        PopUpWidget popup ( Rectangle ( 13, 160, 159, 29 ) );
        popup.Draw();
        pal.GetPalette()->FadeIn ( 0, WINDOW_COLORS, 64, 5 );
        MediaToolkit::GetInstance()->GetClock()->StartTimer ( TMR_CHAPTER, 4000 );
        delayed = true;
        while ( delayed )
        {
            MediaToolkit::GetInstance()->WaitEvents();
        }
        pal.GetPalette()->FadeOut ( 0, WINDOW_COLORS, 64, 5 );
    }
    catch ( Exception &e )
    {
        e.Print ( "Chapter::ShowMap" );
    }
}

int Chapter::Get() const
{
    return number;
}

Zone& Chapter::GetZone()
{
    return zone;
}

void Chapter::Next()
{
    number++;
    zone.Load ( number );
}

void Chapter::Start ( const bool maponly )
{
    try
    {
        if ( !maponly )
        {
            PlayIntro();
            ReadBook ( 1 );
            PlayScene ( 1 );
        }
        ShowMap();
    }
    catch ( Exception &e )
    {
        e.Print ( "Chapter::Start" );
    }
}

void Chapter::KeyPressed ( const KeyboardEvent &kbe )
{
    switch ( kbe.GetKey() )
    {
    case KEY_ESCAPE:
    case KEY_RETURN:
    case KEY_SPACE:
        delayed = false;
        break;
    default:
        break;
    }
}

void Chapter::KeyReleased ( const KeyboardEvent &kbe )
{
    switch ( kbe.GetKey() )
    {
    default:
        break;
    }
}

void Chapter::PointerButtonPressed ( const PointerButtonEvent &pbe )
{
    switch ( pbe.GetButton() )
    {
    case PB_PRIMARY:
        delayed = false;
        break;
    default:
        break;
    }
}

void Chapter::PointerButtonReleased ( const PointerButtonEvent &pbe )
{
    switch ( pbe.GetButton() )
    {
    default:
        break;
    }
}

void Chapter::TimerExpired ( const TimerEvent &te )
{
    if ( te.GetID() == TMR_CHAPTER )
    {
        delayed = false;
    }
}
