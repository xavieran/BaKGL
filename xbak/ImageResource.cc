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
#include "ImageResource.h"

ImageResource::ImageResource()
        : compression ( COMPRESSION_LZW )
        , numImages ( 0 )
{
}

ImageResource::~ImageResource()
{
    Clear();
}

unsigned int ImageResource::GetCompression() const
{
    return compression;
}

void ImageResource::SetCompression ( const unsigned int c )
{
    compression = c;
}

unsigned int ImageResource::GetNumImages() const
{
    return numImages;
}

Image * ImageResource::GetImage ( unsigned int n ) const
{
    return images[n];
}

void ImageResource::Clear()
{
    for ( std::vector<Image*>::iterator it = images.begin(); it != images.end(); ++it )
    {
        delete ( *it );
    }
    images.clear();
}

void ImageResource::Load ( FileBuffer *buffer )
{
    try
    {
        Clear();
        if ( buffer->GetUint16LE() != 0x1066 )
        {
            throw DataCorruption ( __FILE__, __LINE__ );
        }
        compression = ( unsigned int ) buffer->GetUint16LE();
        numImages = ( unsigned int ) buffer->GetUint16LE();
        unsigned int *imageSize = new unsigned int[numImages];
        buffer->Skip ( 2 );
        unsigned int size = buffer->GetUint32LE();
        for ( unsigned int i = 0; i < numImages; i++ )
        {
            imageSize[i] = ( unsigned int ) buffer->GetUint16LE();
            unsigned int flags = ( unsigned int ) buffer->GetUint16LE();
            unsigned int width = ( unsigned int ) buffer->GetUint16LE();
            unsigned int height = ( unsigned int ) buffer->GetUint16LE();
            Image *img = new Image ( width, height, flags );
            images.push_back ( img );
        }
        FileBuffer *decompressed = new FileBuffer ( size );
        buffer->Decompress ( decompressed, compression );
        for ( unsigned int i = 0; i < numImages; i++ )
        {
            FileBuffer *imageBuffer = new FileBuffer ( imageSize[i] );
            imageBuffer->Fill ( decompressed );
            ( images[i] )->Load ( imageBuffer );
            delete imageBuffer;
        }
        delete decompressed;
        delete[] imageSize;
    }
    catch ( Exception &e )
    {
        e.Print ( "ImageResource::Load" );
        throw;
    }
}

unsigned int ImageResource::Save ( FileBuffer *buffer )
{
    try
    {
        buffer->PutUint16LE ( 0x1066 );
        buffer->PutUint16LE ( compression );
        buffer->PutUint16LE ( numImages );
        unsigned int *imageSize = new unsigned int[numImages];
        buffer->PutUint16LE ( 0 );
        unsigned int size = numImages * 4000;
        FileBuffer *decompressed = new FileBuffer ( size );
        for ( unsigned int i = 0; i < numImages; i++ )
        {
            imageSize[i] = ( images[i] )->Save ( decompressed );
        }
        size = decompressed->GetBytesDone();
        buffer->PutUint32LE ( size );
        decompressed->Rewind();
        FileBuffer *compressed = new FileBuffer ( size );
        size = decompressed->Compress ( compressed, compression );
        for ( unsigned int i = 0; i < numImages; i++ )
        {
            Image* img = images[i];
            buffer->PutUint16LE ( imageSize[i] );
            buffer->PutUint16LE ( img->GetFlags() );
            buffer->PutUint16LE ( img->GetWidth() );
            buffer->PutUint16LE ( img->GetHeight() );
        }
        buffer->CopyFrom ( compressed, size );
        delete compressed;
        delete decompressed;
        delete[] imageSize;
        return size;
    }
    catch ( Exception &e )
    {
        e.Print ( "ImageResource::Save" );
        throw;
    }
}
