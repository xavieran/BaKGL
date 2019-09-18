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
#include "MediaToolkit.h"
#include "Text.h"
#include "Widget.h"

TextBlock::TextBlock()
        : words()
        , color ( 0 )
        , shadow ( 0 )
        , shadowXoff ( 0 )
        , shadowYoff ( 0 )
        , italic ( false )
{
}

TextBlock::TextBlock ( const std::string& s, int c, int sh, int shx, int shy, bool it )
        : words ( s )
        , color ( c )
        , shadow ( sh )
        , shadowXoff ( shx )
        , shadowYoff ( shy )
        , italic ( it )
{
}

TextBlock::~TextBlock()
{
}

void TextBlock::SetWords ( const std::string& s )
{
    words = s;
}

void TextBlock::SetColor ( int c )
{
    color = c;
}

void TextBlock::SetShadow ( int sh )
{
    shadow = sh;
}

void TextBlock::SetShadowXOff ( int shx )
{
    shadowXoff = shx;
}

void TextBlock::SetShadowYOff ( int shy )
{
    shadowYoff = shy;
}

void TextBlock::SetItalic ( bool it )
{
    italic = it;
}

unsigned int TextBlock::GetSize() const
{
    return words.size();
}

void TextBlock::AddWords ( const std::string& s )
{
    words += s;
}

void TextBlock::EraseWords ( unsigned int n )
{
    words.erase ( 0, n );
}

const std::string& TextBlock::GetWords() const
{
    return words;
}

int TextBlock::Draw ( int x, int y, int w, int, Font* f, int extraWhiteSpace ) const
{
    if ( extraWhiteSpace < 0 )
    {
        extraWhiteSpace = 0;
    }
    unsigned int i = 1;
    int numSpaces = 0;
    while ( i < words.size() )
    {
        if ( ( words[i - 1] > ' ' ) && ( words[i] == ' ' ) )
        {
            numSpaces++;
        }
        i++;
    }
    int offset = 0;
    int extraOff = 0;
    int space = 0;
    bool prevWasWS = false;
    if ( ( shadow > NO_SHADOW ) && ( ( shadowXoff != 0 ) || ( shadowYoff != 0 ) ) )
    {
        i = 0;
        offset = 0;
        extraOff = 0;
        space = 0;
        prevWasWS = false;
        while ( ( i < words.size() ) && ( offset + extraOff + f->GetWidth ( words[i] - f->GetFirst() ) <= w ) )
        {
            if ( !prevWasWS && ( words[i] == ' ' ) )
            {
                space++;
                extraOff = ( int ) ( ( double ) extraWhiteSpace * ( double ) space / ( double ) numSpaces );
            }
            f->DrawChar ( x + shadowXoff + offset + extraOff, y + shadowYoff, words[i], shadow, italic );
            offset += f->GetWidth ( ( unsigned int ) words[i] - f->GetFirst() );
            prevWasWS = words[i] == ' ';
            i++;
        }
    }
    i = 0;
    offset = 0;
    extraOff = 0;
    space = 0;
    prevWasWS = false;
    while ( ( i < words.size() ) && ( offset + extraOff + f->GetWidth ( words[i] - f->GetFirst() ) <= w ) )
    {
        if ( !prevWasWS && ( words[i] == ' ' ) )
        {
            space++;
            extraOff = ( int ) ( ( double ) extraWhiteSpace * ( double ) space / ( double ) numSpaces );
        }
        f->DrawChar ( x + offset + extraOff, y, words[i], color, italic );
        offset += f->GetWidth ( ( unsigned int ) words[i] - f->GetFirst() );
        prevWasWS = words[i] == ' ';
        i++;
    }
    return offset + extraOff;
}

TextLine::TextLine ( Font *f )
        : font ( f )
        , textBlocks()
        , indent ( 0 )
        , width ( 0 )
{
}

TextLine::~TextLine()
{
    textBlocks.clear();
}

void TextLine::SetIndent ( int i )
{
    indent = i;
}

void TextLine::Clear()
{
    textBlocks.clear();
    width = 0;
}

void TextLine::CopyFirstWord ( std::string& s, std::string& word, int& wordWidth )
{
    wordWidth = 0;
    if ( s[0] == ' ' )
    {
        wordWidth += font->GetWidth ( s[0] - font->GetFirst() );
        s.erase ( 0, 1 );
    }
    unsigned int i = 0;
    while ( ( i < s.size() ) && ( s[i] > ' ' ) )
    {
        wordWidth += font->GetWidth ( s[i] - font->GetFirst() );
        i++;
    }
    word = s.substr ( 0, i );
}

void TextLine::AddWords ( TextBlock& tb, int w )
{
    std::string words = tb.GetWords();
    std::string word;
    int wordWidth = 0;
    TextBlock tmp ( "", tb.GetColor(), tb.GetShadow(), tb.GetShadowXOff(), tb.GetShadowYOff(), tb.GetItalic() );
    bool isFirstWord = true;
    while ( ( !words.empty() ) && ( ( width + wordWidth ) <= w ) )
    {
        CopyFirstWord ( words, word, wordWidth );
        if ( ( width + wordWidth ) <= w )
        {
            if ( !isFirstWord )
            {
                tmp.AddWords ( " " );
                tb.EraseWords ( 1 );
            }
            tmp.AddWords ( word );
            words.erase ( 0, word.size() );
            tb.EraseWords ( word.size() );
            width += wordWidth;
            wordWidth = 0;
            isFirstWord = false;
        }
    }
    textBlocks.push_back ( tmp );
}

void TextLine::Draw ( int x, int y, int w, int h, int extraWhiteSpace ) const
{
    int xoff = indent;
    for ( unsigned int i = 0; i < textBlocks.size(); i++ )
    {
        xoff = textBlocks[i].Draw ( x + xoff, y, w - xoff, h, font, extraWhiteSpace - xoff);
    }
}


Paragraph::Paragraph ( Font *f )
        : font ( f )
        , textBlocks()
        , lines()
        , horAlign ( HA_LEFT )
        , vertAlign ( VA_TOP )
        , indent ( 0 )
{
}

Paragraph::~Paragraph()
{
    textBlocks.clear();
    lines.clear();
}

unsigned int Paragraph::GetSize() const
{
    return textBlocks.size();
}

const std::vector< TextLine >& Paragraph::GetLines() const
{
    return lines;
}

void Paragraph::Clear()
{
    textBlocks.clear();
    lines.clear();
}

void Paragraph::AddTextBlock ( const TextBlock& tb )
{
    textBlocks.push_back ( tb );
}

void Paragraph::SetAlignment ( const HorizontalAlignment ha, const VerticalAlignment va )
{
    horAlign = ha;
    vertAlign = va;
}

void Paragraph::GenerateLines ( int width, int extraIndent )
{
    lines.clear();
    int lineIndent = indent + extraIndent;
    TextLine line ( font );
    line.SetIndent ( lineIndent );
    for ( unsigned int i = 0; i < textBlocks.size(); i++ )
    {
        TextBlock tb = textBlocks[i];
        while ( tb.GetSize() > 0 )
        {
            line.AddWords ( tb, width - lineIndent );
            if ( ( line.GetWidth() > 0 ) && ( tb.GetSize() > 0 ) )
            {
                lines.push_back ( line );
                line.Clear();
                lineIndent = 0;
                line.SetIndent ( lineIndent );
                if ( tb.GetWords() [0] == ' ' )
                {
                    tb.EraseWords ( 1 );
                }
            }
        }
    }
    if ( line.GetWidth() > 0 )
    {
        lines.push_back ( line );
    }
}

void Paragraph::SetIndent ( int i )
{
    indent = i;
}

void Paragraph::Draw ( int x, int &y, int w, int &h, unsigned int& l ) const
{
    while ( ( h >= font->GetHeight() ) && ( l < lines.size() ) )
    {
        int xoff = 0;
        int extraWS = 0;
        if ( ( ( l + 1 ) < lines.size() ) && ( lines[l + 1].GetWidth() > 0 ) )
        {
            switch ( horAlign )
            {
            case HA_LEFT:
                xoff = 0;
                extraWS = 0;
                break;
            case HA_CENTER:
                xoff = ( w - lines[l].GetWidth() ) / 2;
                extraWS = 0;
                break;
            case HA_RIGHT:
                xoff = w - lines[l].GetWidth();
                extraWS = 0;
                break;
            case HA_FILL:
                xoff = 0;
                extraWS = w - lines[l].GetWidth();
                break;
            }
        }
        lines[l].Draw ( x + xoff, y, w, font->GetHeight(), extraWS );
        l++;
        y += font->GetHeight();
        h -= font->GetHeight();
    }
    if ( h < font->GetHeight() )
    {
        h = 0;
    }
}


Text::Text()
        : paragraphs()
        , currentParagraph ( 0 )
        , currentLine ( 0 )
{
}

Text::~Text()
{
    paragraphs.clear();
}

unsigned int Text::GetSize() const
{
    return paragraphs.size();
}

void Text::AddParagraph ( const Paragraph& p )
{
    paragraphs.push_back ( p );
}

void Text::GenerateLines ( int width, int initialIndent )
{
    for ( unsigned int i = 0; i < paragraphs.size(); i++ )
    {
        paragraphs[i].GenerateLines ( width, initialIndent );
        initialIndent = 0;
    }
    currentParagraph = 0;
    currentLine = 0;
}

void Text::DrawPage ( int x, int y, int w, int h )
{
    while ( ( h > 0 ) && ( currentParagraph < paragraphs.size() ) )
    {
        Paragraph p = paragraphs[currentParagraph];
        p.Draw ( x, y, w, h, currentLine );
        if ( currentLine >= p.GetLines().size() )
        {
            currentParagraph++;
            currentLine = 0;
            h -= 4;
            y += 4;
        }
    }
}

void Text::DrawScroll ( int, int, int, int )
{
}
