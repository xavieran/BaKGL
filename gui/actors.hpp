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
        mActorDimensions{},
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
            if (   i ==  9 
                || i == 12 
                || i == 18
                || i == 30)
                actN << "A";

            std::stringstream bmx{};
            bmx << actN.str() << ".BMX";
            mLogger.Debug() << "Loading: " << bmx.str() << " " << pal.str() << std::endl;
            BAK::TextureFactory::AddToTextureStore(
                textures, bmx.str(), pal.str());
        }

        auto& spriteSheet = spriteManager.GetSpriteSheet(mSpriteSheet);
        spriteSheet.LoadTexturesGL(textures);

        for (unsigned i = 1; i < 54; i++)
            mActorDimensions.emplace_back(
                spriteSheet.GetDimensions(i - 1));
    }

    Graphics::SpriteSheetIndex GetSpriteSheet() const
    {
        return mSpriteSheet;
    }

    std::pair<
        Graphics::TextureIndex,
        glm::vec2>
    GetActor(unsigned actor) const
    {
        unsigned index = actor - 1;
        assert(index < mActorDimensions.size());
        return std::make_pair(
            index,
            mActorDimensions[index]);
    }

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::vector<glm::vec2> mActorDimensions;

    const Logging::Logger& mLogger;
};

}
