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
#include "BookResource.h"

BookResource::BookResource()
        : pages()
{
}

BookResource::~BookResource()
{
    Clear();
}

unsigned int BookResource::GetSize() const
{
    return pages.size();
}

const PageData& BookResource::GetPage ( const unsigned int i ) const
{
    return pages[i];
}

void BookResource::Clear()
{
    pages.clear();
}

void BookResource::Load ( FileBuffer *buffer )
{
    try
    {
        Clear();
        buffer->Skip ( 4 ); // file size
        unsigned int numPages = buffer->GetUint16LE();
        unsigned int *pageOffset = new unsigned int [numPages];
        for ( unsigned int i = 0; i < numPages; i++ )
        {
            pageOffset[i] = buffer->GetUint32LE();
        }
        for ( unsigned int i = 0; i < numPages; i++ )
        {
            buffer->Seek ( 4 + pageOffset[i] );
            PageData pd;
            pd.xpos = buffer->GetSint16LE();
            pd.ypos = buffer->GetSint16LE();
            pd.width = buffer->GetSint16LE();
            pd.height = buffer->GetSint16LE();
            pd.number = buffer->GetUint16LE();
            pd.id = buffer->GetUint16LE();
            pd.prevId = buffer->GetUint16LE();
            buffer->Skip ( 2 );
            pd.nextId = buffer->GetUint16LE();
            pd.flag = buffer->GetUint16LE();
            unsigned int numDecorations = buffer->GetUint16LE();
            unsigned int numFirstLetters = buffer->GetUint16LE();
            pd.showNumber = buffer->GetUint16LE() > 0;
            buffer->Skip ( 30 );
            for ( unsigned int j = 0; j < numDecorations; j++ )
            {
                ImageInfo info;
                info.xpos = buffer->GetSint16LE();
                info.ypos = buffer->GetSint16LE();
                info.id = buffer->GetUint16LE();
                info.flag = buffer->GetUint16LE();
                pd.decorations.push_back ( info );
            }
            for ( unsigned int j = 0; j < numFirstLetters; j++ )
            {
                ImageInfo info;
                info.xpos = buffer->GetSint16LE();
                info.ypos = buffer->GetSint16LE();
                info.id = buffer->GetUint16LE();
                info.flag = buffer->GetUint16LE();
                pd.firstLetters.push_back ( info );
            }
            bool endOfPage = false;
            TextInfo ti;
            ti.paragraph = true;
            ti.italic = false;
            while ( !endOfPage && !buffer->AtEnd() )
            {
                unsigned char c = buffer->GetUint8();
                if ( ( c & 0xf0 ) == 0xf0 )
                {
                    switch ( c )
                    {
                    case 0xf0:
                        endOfPage = true;
                        break;
                    case 0xf1:
                        // end of paragraph
                        buffer->Skip ( 16 );
                        break;
                    case 0xf2:
                        break;
                    case 0xf3:
                        break;
                    case 0xf4:
                        buffer->Skip ( 8 );
                        switch ( buffer->GetUint16LE() )
                        {
                        case 1:
                            ti.italic = false;
                            break;
                        case 5:
                            ti.italic = true;
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                    pd.textBlocks.push_back ( ti );
                    ti.italic = false;
                    ti.txt.clear();
                }
                else
                {
                    ti.txt.push_back ( c );
                }
            }
            pd.textBlocks.push_back ( ti );
            pages.push_back ( pd );
        }
        delete[] pageOffset;
    }
    catch ( Exception &e )
    {
        e.Print ( "BookResource::Load" );
        throw;
    }
}

unsigned int BookResource::Save ( FileBuffer *buffer )
{
    try
    {
        // TODO
        buffer = buffer;
        return 0;
    }
    catch ( Exception &e )
    {
        e.Print ( "BookResource::Save" );
        throw;
    }
}
