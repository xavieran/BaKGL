#pragma once

#include "graphics/texture.hpp"

#include <array>

namespace BAK {

class FileBuffer;

struct Glyph
{
    static constexpr auto MAX_FONT_HEIGHT = 16;
    Glyph(
        unsigned width,
        unsigned height,
        std::array<std::uint16_t, MAX_FONT_HEIGHT> points);

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
        Graphics::TextureStore textures);

    char GetIndex(char c) const;
    const Graphics::TextureStore& GetCharacters();

    std::size_t GetSpace() const;

    char GetFirstChar() const;
    unsigned GetWidth(char c) const;
    unsigned GetHeight() const;

private:
    int mFirstChar;
    unsigned mHeight;
    Graphics::TextureStore mCharacters;
};

Font LoadFont(FileBuffer& fb);

}
