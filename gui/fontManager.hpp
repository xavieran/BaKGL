#pragma once

#include "bak/font.hpp"

#include "graphics/glm.hpp"
#include "graphics/sprites.hpp"

namespace Gui {

class Font
{
public:
    Font(
        const std::string& font,
        Graphics::SpriteManager& spriteManager)
    :
        mFont{std::invoke([&]{
            auto fb = FileBufferFactory::Get().CreateDataBuffer(font);
            return BAK::LoadFont(fb);
        })},
        mSpriteSheet{spriteManager.AddSpriteSheet()}
    {
        spriteManager.GetSpriteSheet(mSpriteSheet)
            .LoadTexturesGL(mFont.GetCharacters());
    }

    const auto& GetFont() const { return mFont; }
    const auto& GetSpriteSheet() const { return mSpriteSheet; }

private:
    BAK::Font mFont;
    Graphics::SpriteSheetIndex mSpriteSheet;
};

class FontManager
{
public:
    FontManager(
        Graphics::SpriteManager& spriteManager)
    :
        mAlien{"ALIEN.FNT", spriteManager},
        mBook{"BOOK.FNT", spriteManager},
        mGame{"GAME.FNT", spriteManager},
        mPuzzle{"PUZZLE.FNT", spriteManager},
        mSpell{"SPELL.FNT", spriteManager}
    {}

    const Font& GetAlienFont() const { return mAlien; }
    const Font& GetBookFont() const { return mBook; }
    const Font& GetGameFont() const { return mGame; }
    const Font& GetPuzzleFont() const { return mPuzzle; }
    const Font& GetSpellFont() const { return mSpell; }

private:
    Font mAlien;
    Font mBook;
    Font mGame;
    Font mPuzzle;
    Font mSpell;
};

}


