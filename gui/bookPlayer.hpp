#pragma once

#include "bak/book.hpp"

#include "graphics/sprites.hpp"

#include "gui/core/widget.hpp"
#include "gui/textBox.hpp"

#include <optional>

namespace Gui {

class Backgrounds;
class Font;

class BookPlayer
{
    
public:
    BookPlayer(
        Graphics::SpriteManager& spriteManager,
        const Font& font,
        const Backgrounds& background,
        std::function<void()> finishedBook);

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

    std::optional<BAK::Book> mBook{};
    unsigned mCurrentPage{};
    std::string mText{};
    std::function<void()> mFinishedBook{};
};

}
