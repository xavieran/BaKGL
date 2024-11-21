#include "gui/fontManager.hpp"

#include "bak/font.hpp"
#include "bak/fileBufferFactory.hpp"

#include "graphics/sprites.hpp"

namespace Gui {

Font::Font(
    const std::string& font,
    Graphics::SpriteManager& spriteManager)
:
    mFont{std::invoke([&]{
        auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(font);
        return BAK::LoadFont(fb);
    })},
    mSpriteSheet{spriteManager.AddSpriteSheet()}
{
    spriteManager.GetSpriteSheet(mSpriteSheet)
        .LoadTexturesGL(mFont.GetCharacters());
}

const BAK::Font& Font::GetFont() const { return mFont; }
const Graphics::SpriteSheetIndex& Font::GetSpriteSheet() const { return mSpriteSheet; }


FontManager::FontManager(
    Graphics::SpriteManager& spriteManager)
:
    mAlien{"ALIEN.FNT", spriteManager},
    mBook{"BOOK.FNT", spriteManager},
    mGame{"GAME.FNT", spriteManager},
    mPuzzle{"PUZZLE.FNT", spriteManager},
    mSpell{"SPELL.FNT", spriteManager}
{}

const Font& FontManager::GetAlienFont() const { return mAlien; }
const Font& FontManager::GetBookFont() const { return mBook; }
const Font& FontManager::GetGameFont() const { return mGame; }
const Font& FontManager::GetPuzzleFont() const { return mPuzzle; }
const Font& FontManager::GetSpellFont() const { return mSpell; }

}


