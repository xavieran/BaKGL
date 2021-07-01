#pragma once

#include "graphics/texture.hpp"

#include "xbak/FileBuffer.h"

#include <vector>

namespace BAK {

struct Glyph
{
    static constexpr auto MAX_FONT_HEIGHT = 16;
    unsigned mWidth;
    unsigned mHeight;
    std::array<std::uint16_t, MAX_FONT_HEIGHT> mPoints;
};

Graphics::Texture GlyphToTexture(const Glyph&);

struct Font
{
    auto GetIndex(char c) const { return c - mFirstChar; }
    auto GetWidth(char c) const
    {
        return mCharacters.GetTexture(GetIndex(c)).GetWidth();
    }

    int mFirstChar;
    unsigned mHeight;
    Graphics::TextureStore mCharacters;
    //std::vector<Glyph> mItalicGlyphs;
};

Font LoadFont(FileBuffer& fb);

}
