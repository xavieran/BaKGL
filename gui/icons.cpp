#include "gui/icons.hpp"

#include "bak/textureFactory.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include <glm/glm.hpp>

#include <tuple>

namespace Gui {

Icons::Icons(
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
            mButtonIconsDims.emplace_back(t.GetDims());
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

        mInventoryLockOffset = textures.size();
        BAK::TextureFactory::AddToTextureStore(
            textures, "INVLOCK.BMX", "OPTIONS.PAL");

        mFullMapIconOffset = textures.size();
        BAK::TextureFactory::AddToTextureStore(
            textures, "FMAP_ICN.BMX", "FULLMAP.PAL");

        mTeleportIconOffset = textures.size();
        BAK::TextureFactory::AddToTextureStore(
            textures, "TELEPORT.BMX", "TELEPORT.PAL");

        mEncampIconOffset = textures.size();
        BAK::TextureFactory::AddToTextureStore(
            textures, "ENCAMP.BMX", "OPTIONS.PAL");

        mCastIconOffset = textures.size();
        BAK::TextureFactory::AddToTextureStore(
            textures, "CASTFACE.BMX", "OPTIONS.PAL");

        for (const auto& t : textures.GetTextures())
        {
            mInventoryIconsDims.emplace_back(t.GetDims());
        }

        auto& spriteSheet = spriteManager.GetSpriteSheet(mInventoryIconsSpriteSheet);
        spriteSheet.LoadTexturesGL(textures);
    }
}

Icons::IconInfo Icons::GetButton(unsigned i) const
{
    ASSERT(i < mButtonIconsDims.size());
    return std::make_tuple(
        mButtonIconsSpriteSheet,
        Graphics::TextureIndex{i},
        mButtonIconsDims[i]);
}

Icons::IconInfo Icons::GetPressedButton(unsigned i) const
{
    const auto index = i + mPressedOffset;
    ASSERT(index < mButtonIconsDims.size());
    return std::make_tuple(
        mButtonIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mButtonIconsDims[index]);
}

Icons::IconInfo Icons::GetInventoryIcon(unsigned i) const
{
    ASSERT(i < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{i},
        mInventoryIconsDims[i]);
}

Icons::IconInfo Icons::GetStippledBorderHorizontal() const
{
    return GetInventoryIcon(145);
}

Icons::IconInfo Icons::GetStippledBorderVertical() const
{
    return GetInventoryIcon(146);
}

Icons::IconInfo Icons::GetCharacterHead(unsigned i) const
{
    const auto index = i + mHeadsOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetCompass() const
{
    const auto index = mCompassOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetInventoryMiscIcon(unsigned i) const
{
    const auto index = i + mInventoryMiscOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetInventoryModifierIcon(unsigned i) const
{
    const auto index = i + sInventoryModsOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetInventoryLockIcon(unsigned i) const
{
    const auto index = i + mInventoryLockOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetFullMapIcon(unsigned i) const
{
    const auto index = i + mFullMapIconOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetTeleportIcon(unsigned i) const
{
    const auto index = i + mTeleportIconOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetEncampIcon(unsigned i) const
{
    const auto index = i + mEncampIconOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

Icons::IconInfo Icons::GetCastIcon(unsigned i) const
{
    const auto index = i + mCastIconOffset;
    ASSERT(index < mInventoryIconsDims.size());
    return std::make_tuple(
        mInventoryIconsSpriteSheet,
        Graphics::TextureIndex{index},
        mInventoryIconsDims[index]);
}

std::size_t Icons::GetSize() const
{
    return mInventoryIconsDims.size();
}
}
