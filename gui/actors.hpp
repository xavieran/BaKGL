#pragma once

#include "bak/textureFactory.hpp"

#include "com/assert.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/core/widget.hpp"

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
        mActorADimensions{},
        mLogger{Logging::LogState::GetLogger("Gui::Actors")}
    {
        auto textures = Graphics::TextureStore{};

        unsigned textureIndex = 0;
        for (unsigned i = 1; i < 54; i++)
        {
            std::stringstream actN{};
            actN << "ACT0" << std::setw(2) << std::setfill('0') << i;

            std::stringstream pal{};
            pal << actN.str() << ".PAL";

            {
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
                const auto [x, y] = textures.GetTexture(textureIndex).GetDims();
                mActorDimensions.emplace_back(std::make_pair(textureIndex, glm::vec2{x, y}));
                textureIndex++;
            }

            // Add all alternates ... "A"
            if (i  < 7)
            {
                actN << "A";
                std::stringstream bmx{};
                bmx << actN.str() << ".BMX";
                mLogger.Debug() << "Loading alternate: " << bmx.str() << " " << pal.str() << std::endl;
                BAK::TextureFactory::AddToTextureStore(
                    textures, bmx.str(), pal.str());
                const auto [x, y] = textures.GetTexture(textureIndex).GetDims();
                mActorADimensions.emplace(
                    i,
                    std::make_pair(textureIndex, glm::vec2{x, y}));
                textureIndex++;
            }
        }

        auto& spriteSheet = spriteManager.GetSpriteSheet(mSpriteSheet);
        spriteSheet.LoadTexturesGL(textures);
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
        mLogger.Info() << "Get Actor: " << actor << "\n";
        unsigned index = actor - 1;
        ASSERT(index < mActorDimensions.size());
        return mActorDimensions[index];
    }

    std::pair<
        Graphics::TextureIndex,
        glm::vec2>
    GetActorA(unsigned actor) const
    {
        ASSERT(mActorADimensions.contains(actor));
        return mActorADimensions.find(actor)->second;
    }

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::vector<std::pair<Graphics::TextureIndex, glm::vec2>> mActorDimensions;
    std::unordered_map<
        unsigned,
        std::pair<Graphics::TextureIndex, glm::vec2>> mActorADimensions;

    const Logging::Logger& mLogger;
};

}
