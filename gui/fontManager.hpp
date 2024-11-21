#pragma once

#include "bak/font.hpp"

#include "graphics/sprites.hpp"

namespace Gui {

class Font
{
public:
    Font(
        const std::string& font,
        Graphics::SpriteManager& spriteManager);
    
    const BAK::Font& GetFont() const;
    const Graphics::SpriteSheetIndex& GetSpriteSheet() const;

private:
    BAK::Font mFont;
    Graphics::SpriteSheetIndex mSpriteSheet;
};

class FontManager
{
public:
    FontManager(
        Graphics::SpriteManager& spriteManager);
    const Font& GetAlienFont() const;
    const Font& GetBookFont() const;
    const Font& GetGameFont() const;
    const Font& GetPuzzleFont() const;
    const Font& GetSpellFont() const;

private:
    Font mAlien;
    Font mBook;
    Font mGame;
    Font mPuzzle;
    Font mSpell;
};

}


