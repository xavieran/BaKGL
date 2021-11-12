#pragma once

#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class Backgrounds
{
public:

    Backgrounds(
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

        for (const auto& [scx, pal] : {
            std::make_pair("ENCAMP.SCX", "OPTIONS.PAL"),
            std::make_pair("FRAME.SCX", "OPTIONS.PAL"),
            std::make_pair("CONT2.SCX", "CONTENTS.PAL"),
            std::make_pair("FULLMAP.SCX", "FULLMAP.PAL"),
            std::make_pair("DIALOG.SCX", "INVENTOR.PAL"),
            std::make_pair("INVENTOR.SCX", "INVENTOR.PAL"),
            std::make_pair("OPTIONS0.SCX", "OPTIONS.PAL"),
            std::make_pair("OPTIONS1.SCX", "OPTIONS.PAL"),
            std::make_pair("OPTIONS2.SCX", "OPTIONS.PAL"),
            std::make_pair("PUZZLE.SCX", "PUZZLE.PAL")
        })
        {
            AddScreen(scx, pal);
        }

        spriteManager
            .GetSpriteSheet(mSpriteSheet)
            .LoadTexturesGL(textures);
    }


    Graphics::SpriteSheetIndex GetSpriteSheet() const
    {
        return mSpriteSheet;
    }

    Graphics::TextureIndex GetScreen(const std::string& scx) const
    {
        ASSERT(mScxToSprite.contains(scx));
        const auto ti = *mScxToSprite.find(scx);
        return ti.second;
    }

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::unordered_map<std::string, Graphics::TextureIndex> mScxToSprite;

    const Logging::Logger& mLogger;
};

}
