#pragma once

#include "graphics/texture.hpp"

#include "xbak/FileBuffer.h"

#include <iostream>
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

class Font
{
public:
    Font(
        int firstChar,
        unsigned height,
        Graphics::TextureStore textures)
    :
        mFirstChar{firstChar},
        mHeight{height},
        mCharacters{textures}
    {}

    char GetIndex(char c) const
    {
        if (!(mFirstChar <= c))
        {
            std::cout << "BAD:{" << std::dec << +c << "}\n";
        }
        assert(mFirstChar <= c);
        return c - mFirstChar;
    }

    const auto& GetCharacters(){ return mCharacters; }

    std::size_t GetSpace() const
    {
        // The width of 'a'
        return static_cast<float>(
            mCharacters.GetTexture(0).GetWidth());
    }

    char GetFirstChar() const { return mFirstChar; }
    unsigned GetWidth(char c) const
    {
        return mCharacters.GetTexture(GetIndex(c)).GetWidth();
    }

    unsigned GetHeight() const
    {
        return mHeight;
    }

private:
    int mFirstChar;
    unsigned mHeight;
    Graphics::TextureStore mCharacters;
};

Font LoadFont(FileBuffer& fb);

}
