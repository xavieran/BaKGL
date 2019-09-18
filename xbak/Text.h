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

#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Font.h"

typedef enum
{
    HA_LEFT,
    HA_CENTER,
    HA_RIGHT,
    HA_FILL
} HorizontalAlignment;

typedef enum
{
    VA_TOP,
    VA_CENTER,
    VA_BOTTOM
} VerticalAlignment;

class TextBlock
{
private:
    std::string words;
    int color;
    int shadow;
    int shadowXoff;
    int shadowYoff;
    bool italic;
public:
    TextBlock();
    TextBlock ( const std::string& s, int c, int sh, int shx, int shy, bool it );
    ~TextBlock();
    void SetColor ( int c );
    int GetColor() const
    {
        return color;
    }
    void SetShadow ( int sh );
    int GetShadow() const
    {
        return shadow;
    }
    void SetShadowXOff ( int shx );
    int GetShadowXOff() const
    {
        return shadowXoff;
    }
    void SetShadowYOff ( int shy );
    int GetShadowYOff() const
    {
        return shadowYoff;
    }
    void SetItalic ( bool it );
    int GetItalic() const
    {
        return italic;
    }
    void SetWords ( const std::string& s );
    const std::string& GetWords() const;
    void AddWords ( const std::string& s );
    void EraseWords ( unsigned int n );
    unsigned int GetSize() const;
    int Draw ( int x, int y, int w, int h, Font *f, int extraWhiteSpace = 0 ) const;
};

class TextLine
{
private:
    Font *font;
    std::vector<TextBlock> textBlocks;
    int indent;
    int width;
    void CopyFirstWord ( std::string& s, std::string& word, int& wordWidth );
public:
    TextLine ( Font *f );
    ~TextLine();
    void SetIndent ( int i );
    void Clear();
    void AddWords ( TextBlock& tb, int w );
    int GetWidth() const
    {
        return width;
    }
    void Draw ( int x, int y, int w, int h, int extraWhiteSpace ) const;
};

class Paragraph
{
private:
    Font *font;
    std::vector<TextBlock> textBlocks;
    std::vector<TextLine> lines;
    HorizontalAlignment horAlign;
    VerticalAlignment vertAlign;
    int indent;
public:
    Paragraph ( Font *f );
    ~Paragraph();
    unsigned int GetSize() const;
    const std::vector<TextLine>& GetLines() const;
    void Clear();
    void AddTextBlock ( const TextBlock& tb );
    void SetAlignment ( const HorizontalAlignment ha, const VerticalAlignment va );
    void SetIndent ( int i );
    void GenerateLines ( int width, int extraIndent );
    void Draw ( int x, int &y, int w, int &h, unsigned int& l ) const;
};

class Text
{
private:
    std::vector<Paragraph> paragraphs;
    unsigned int currentParagraph;
    unsigned int currentLine;
public:
    Text();
    ~Text();
    unsigned int GetSize() const;
    void AddParagraph ( const Paragraph& p );
    void GenerateLines ( int width, int initialIndent );
    void DrawPage ( int x, int y, int w, int h );
    void DrawScroll ( int x, int y, int w, int h );
};

#endif
