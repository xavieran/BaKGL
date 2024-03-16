#pragma once

#include "bak/book.hpp"

#include "com/logger.hpp"

#include "graphics/guiTypes.hpp"
#include "graphics/sprites.hpp"

#include "gui/animatorStore.hpp"
#include "gui/scene.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/core/widget.hpp"
#include "gui/textBox.hpp"

namespace Gui {

class BookPlayer
{
    
public:
    BookPlayer(
        Graphics::SpriteManager& spriteManager,
        const Font& font,
        const Backgrounds& background);

    void PlayBook(std::string);
    void AdvancePage();

    Widget* GetBackground();

private:
    void RenderPage(const BAK::Page&);

    Graphics::SpriteManager& mSpriteManager;
    Graphics::SpriteManager::TemporarySpriteSheet mSpriteSheet;
    const Font& mFont;
    Graphics::TextureStore mTextures;

    Widget mBackground;
    TextBox mTextBox;
    std::vector<Widget> mImages;

    std::optional<BAK::Book> mBook;
    unsigned mCurrentPage;
    std::string mText;
};

}
