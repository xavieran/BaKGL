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

class Actors
{
public:

    Actors(
        Graphics::SpriteManager& spriteManager)
    :
        mSpriteSheet{spriteManager.AddSpriteSheet()},
        mLogger{Logging::LogState::GetLogger("Gui::Actors")}
    {
        auto textures = Graphics::TextureStore{};

        for (unsigned i = 1; i < 54; i++)
        {
            std::stringstream actN{};
            actN << "ACT0" << std::setw(2) << std::setfill('0') << i;

            std::stringstream pal{};
            pal << actN.str() << ".PAL";
            
            // Add all alternates ... "A"
            if (   i == 30 
                || i == 18
                || i == 12
                || i == 9)
                actN << "A";

            std::stringstream bmx{};
            bmx << actN.str() << ".BMX";
            mLogger.Debug() << "Loading: " << bmx.str() << " " << pal.str() << std::endl;
            BAK::TextureFactory::AddToTextureStore(
                textures, bmx.str(), pal.str());
        }

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

    std::pair<
        Graphics::SpriteSheetIndex,
        Graphics::TextureIndex>
    GetActor(unsigned i) const
    {
        return std::make_pair(mSpriteSheet, i - 1);
    }

private:
    Graphics::SpriteSheetIndex mSpriteSheet;

    const Logging::Logger& mLogger;
};

}
