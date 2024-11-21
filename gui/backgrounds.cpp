#include "gui/backgrounds.hpp"

#include "bak/textureFactory.hpp"

#include "com/logger.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"

#include <glm/glm.hpp>

namespace Gui {

Backgrounds::Backgrounds(
    Graphics::SpriteManager& spriteManager)
:
    mSpriteSheet{spriteManager.AddSpriteSheet()},
    mScxToSprite{},
    mLogger{Logging::LogState::GetLogger("Gui::Backgrounds")}
{
    unsigned i = 0;
    auto textures = Graphics::TextureStore{};

    const auto AddScreen = [this, &i, &textures](
        const std::string& scx,
        const std::string& pal)
    {
        mScxToSprite.emplace(
            std::make_pair(scx, i++));
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, scx, pal);
    };

    const auto AddBackground = [this, &i, &textures](
        const std::string& bmx,
        const std::string& pal)
    {
        mScxToSprite.emplace(
            std::make_pair(bmx, i++));
        BAK::TextureFactory::AddToTextureStore(
            textures, bmx, pal);
    };

    for (const auto& [scx, pal] : {
        std::make_pair("ENCAMP.SCX", "OPTIONS.PAL"),
        std::make_pair("FRAME.SCX", "OPTIONS.PAL"),
        std::make_pair("CONTENTS.SCX", "CONTENTS.PAL"),
        std::make_pair("CONT2.SCX", "CONTENTS.PAL"),
        std::make_pair("FULLMAP.SCX", "FULLMAP.PAL"),
        std::make_pair("RIFTMAP.SCX", "OPTIONS.PAL"),
        std::make_pair("INT_BORD.SCX", "FULLMAP.PAL"),
        std::make_pair("DIALOG.SCX", "INVENTOR.PAL"),
        std::make_pair("INVENTOR.SCX", "INVENTOR.PAL"),
        std::make_pair("OPTIONS0.SCX", "OPTIONS.PAL"),
        std::make_pair("OPTIONS1.SCX", "OPTIONS.PAL"),
        std::make_pair("OPTIONS2.SCX", "OPTIONS.PAL"),
        std::make_pair("PUZZLE.SCX", "PUZZLE.PAL"),
        std::make_pair("C42.SCX", "TELEPORT.PAL"),
        std::make_pair("CAST.SCX", "OPTIONS.PAL"),
        std::make_pair("CFRAME.SCX", "OPTIONS.PAL"),
        std::make_pair("BOOK.SCX", "BOOK.PAL")
    })
    {
        AddScreen(scx, pal);
    }

    // Add this screen and cut out the center so we can use
    // it in the main view without any scissoring
    {
        mScxToSprite.emplace(std::make_pair("DIALOG_BG_MAIN.SCX", i++));
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "DIALOG.SCX", "OPTIONS.PAL");
        auto& tex = textures.GetTexture(i - 1);
        for (auto& pixel : tex.GetTexture())
        {
            if (pixel.a == 0)
            {
                pixel = Color::black;
            }
        }
        for (unsigned x = 13; x < (294 + 13); x++)
        {
            for (unsigned y = (200 - 13); y > (200 - (100 + 13)); y--)
            {
                tex.SetPixel(x, y, glm::vec4{0});
            }
        }
        for (unsigned x = 13; x < (294 + 13); x++)
        {
            tex.SetPixel(x, (200 - (100 + 13)), Color::frameMaroon);
            tex.SetPixel(x, (200 - 13), Color::frameMaroon);
        }
            for (unsigned y = (200 - 13); y > (200 - (100 + 14)); y--)
        {
            tex.SetPixel(12, y, Color::frameMaroon);
            tex.SetPixel(294 + 13, y, Color::frameMaroon);
        }
    }

    for (const auto& [bmx, pal] : {
        std::make_pair("TELEPORT.BMX", "TELEPORT.PAL"),
    })
    {
        AddBackground(bmx, pal);
    }

    spriteManager
        .GetSpriteSheet(mSpriteSheet)
        .LoadTexturesGL(textures);
}


Graphics::SpriteSheetIndex Backgrounds::GetSpriteSheet() const
{
    return mSpriteSheet;
}

Graphics::TextureIndex Backgrounds::GetScreen(const std::string& scx) const
{
    ASSERT(mScxToSprite.contains(scx));
    return mScxToSprite.find(scx)->second;
}

}
