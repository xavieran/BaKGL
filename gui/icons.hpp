#pragma once

#include "bak/textureFactory.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <tuple>
#include <variant>

namespace Gui {

class Icons
{
public:
    using IconInfo = std::tuple<
        Graphics::SpriteSheetIndex,
        Graphics::TextureIndex,
        glm::vec2>;

    Icons(
        Graphics::SpriteManager& spriteManager)
    :
        mButtonIconsSpriteSheet{spriteManager.AddSpriteSheet()},
        mInventoryIconsSpriteSheet{spriteManager.AddSpriteSheet()},
        mLogger{Logging::LogState::GetLogger("Gui::Icons")}
    {
        {
            auto textures = Graphics::TextureStore{};
            BAK::TextureFactory::AddToTextureStore(
                textures, "BICONS1.BMX", "OPTIONS.PAL");

            mPressedOffset = textures.size();

            BAK::TextureFactory::AddToTextureStore(
                textures, "BICONS2.BMX", "OPTIONS.PAL");

            for (const auto& t : textures.GetTextures())
            {
                const auto [w, h] = t.GetDims();
                mButtonIconsDims.emplace_back(w, h);
            }

            auto& spriteSheet = spriteManager.GetSpriteSheet(mButtonIconsSpriteSheet);
            spriteSheet.LoadTexturesGL(textures);
        }

        {
            auto textures = Graphics::TextureStore{};
            BAK::TextureFactory::AddToTextureStore(
                textures, "INVSHP1.BMX", "OPTIONS.PAL");

            BAK::TextureFactory::AddToTextureStore(
                textures, "INVSHP2.BMX", "OPTIONS.PAL");

            mHeadsOffset = textures.size();

            BAK::TextureFactory::AddToTextureStore(
                textures, "HEADS.BMX", "OPTIONS.PAL");

            mCompassOffset = textures.size();
            BAK::TextureFactory::AddToTextureStore(
                textures, "COMPASS.BMX", "OPTIONS.PAL");

            mInventoryMiscOffset = textures.size();
            BAK::TextureFactory::AddToTextureStore(
                textures, "INVMISC.BMX", "OPTIONS.PAL");


            for (const auto& t : textures.GetTextures())
            {
                const auto [w, h] = t.GetDims();
                mInventoryIconsDims.emplace_back(w, h);
            }

            auto& spriteSheet = spriteManager.GetSpriteSheet(mInventoryIconsSpriteSheet);
            spriteSheet.LoadTexturesGL(textures);
        }
    }

    IconInfo GetButton(unsigned i) const
    {
        assert(i < mButtonIconsDims.size());
        return std::make_tuple(
            mButtonIconsSpriteSheet,
            Graphics::TextureIndex{i},
            mButtonIconsDims[i]);
    }

    IconInfo GetPressedButton(unsigned i) const
    {
        const auto index = i + mPressedOffset;
        assert(index < mButtonIconsDims.size());
        return std::make_tuple(
            mButtonIconsSpriteSheet,
            Graphics::TextureIndex{index},
            mButtonIconsDims[index]);
    }

    IconInfo GetInventoryIcon(unsigned i) const
    {
        assert(i < mInventoryIconsDims.size());
        return std::make_tuple(
            mInventoryIconsSpriteSheet,
            Graphics::TextureIndex{i},
            mInventoryIconsDims[i]);
    }

    IconInfo GetStippledBorderHorizontal() const
    {
        return GetInventoryIcon(145);
    }

    IconInfo GetStippledBorderVertical() const
    {
        return GetInventoryIcon(146);
    }

    IconInfo GetCharacterHead(unsigned i) const
    {
        const auto index = i + mHeadsOffset;
        assert(index < mInventoryIconsDims.size());
        return std::make_tuple(
            mInventoryIconsSpriteSheet,
            Graphics::TextureIndex{index},
            mInventoryIconsDims[index]);
    }

    IconInfo GetCompass() const
    {
        const auto index = mCompassOffset;
        assert(index < mInventoryIconsDims.size());
        return std::make_tuple(
            mInventoryIconsSpriteSheet,
            Graphics::TextureIndex{index},
            mInventoryIconsDims[index]);
    }

    IconInfo GetInventoryMiscIcon(unsigned i) const
    {
        const auto index = i + mInventoryMiscOffset;
        assert(index < mInventoryIconsDims.size());
        return std::make_tuple(
            mInventoryIconsSpriteSheet,
            Graphics::TextureIndex{index},
            mInventoryIconsDims[index]);
    }

private:
    Graphics::SpriteSheetIndex mButtonIconsSpriteSheet;
    unsigned mPressedOffset;
    std::vector<glm::vec2> mButtonIconsDims;

    Graphics::SpriteSheetIndex mInventoryIconsSpriteSheet;
    unsigned mHeadsOffset;
    unsigned mCompassOffset;
    unsigned mInventoryMiscOffset;
    std::vector<glm::vec2> mInventoryIconsDims;

    const Logging::Logger& mLogger;
};

}
