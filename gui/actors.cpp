#include "gui/actors.hpp"

#include "bak/textureFactory.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/sprites.hpp"
#include "graphics/texture.hpp"

#include <glm/glm.hpp>

#include <iostream>

namespace Gui {

Actors::Actors(
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
            mLogger.Spam() << "Loading: " << bmx.str() << " " << pal.str() << std::endl;
            BAK::TextureFactory::AddToTextureStore(
                textures, bmx.str(), pal.str());
            const auto dims = textures.GetTexture(textureIndex).GetDims();
            mActorDimensions.emplace_back(std::make_pair(textureIndex, dims));
            textureIndex++;
        }

        // Add all alternates ... "A"
        if (i  < 7)
        {
            actN << "A";
            std::stringstream bmx{};
            bmx << actN.str() << ".BMX";
            mLogger.Spam() << "Loading alternate: " << bmx.str() << " " << pal.str() << std::endl;
            BAK::TextureFactory::AddToTextureStore(
                textures, bmx.str(), pal.str());
            const auto dims = textures.GetTexture(textureIndex).GetDims();
            mActorADimensions.emplace(
                i,
                std::make_pair(textureIndex, dims));
            textureIndex++;
        }
    }

    auto& spriteSheet = spriteManager.GetSpriteSheet(mSpriteSheet);
    spriteSheet.LoadTexturesGL(textures);
}

Graphics::SpriteSheetIndex Actors::GetSpriteSheet() const
{
    return mSpriteSheet;
}

std::pair<
    Graphics::TextureIndex,
    glm::vec2>
Actors::GetActor(unsigned actor) const
{
    mLogger.Spam() << "Get Actor: " << actor << "\n";
    unsigned index = actor - 1;
    ASSERT(index < mActorDimensions.size());
    return mActorDimensions[index];
}

std::pair<
    Graphics::TextureIndex,
    glm::vec2>
Actors::GetActorA(unsigned actor) const
{
    ASSERT(mActorADimensions.contains(actor));
    return mActorADimensions.find(actor)->second;
}


}
