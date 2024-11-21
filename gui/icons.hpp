#pragma once

#include "graphics/sprites.hpp"

#include <glm/glm.hpp>

#include <tuple>

namespace Gui {

class Icons
{
public:
    using IconInfo = std::tuple<
        Graphics::SpriteSheetIndex,
        Graphics::TextureIndex,
        glm::vec2>;

    static constexpr auto sInventoryModsOffset = 132;

    Icons(
        Graphics::SpriteManager& spriteManager);

    IconInfo GetButton(unsigned i) const;
    IconInfo GetPressedButton(unsigned i) const;
    IconInfo GetInventoryIcon(unsigned i) const;
    IconInfo GetStippledBorderHorizontal() const;
    IconInfo GetStippledBorderVertical() const;
    IconInfo GetCharacterHead(unsigned i) const;
    IconInfo GetCompass() const;
    IconInfo GetInventoryMiscIcon(unsigned i) const;
    IconInfo GetInventoryModifierIcon(unsigned i) const;
    IconInfo GetInventoryLockIcon(unsigned i) const;
    IconInfo GetFullMapIcon(unsigned i) const;
    IconInfo GetTeleportIcon(unsigned i) const;
    IconInfo GetEncampIcon(unsigned i) const;
    IconInfo GetCastIcon(unsigned i) const;
    std::size_t GetSize() const;

private:
    Graphics::SpriteSheetIndex mButtonIconsSpriteSheet;
    unsigned mPressedOffset;
    std::vector<glm::vec2> mButtonIconsDims;

    Graphics::SpriteSheetIndex mInventoryIconsSpriteSheet;
    unsigned mHeadsOffset;
    unsigned mCompassOffset;
    unsigned mInventoryMiscOffset;
    unsigned mInventoryLockOffset;
    unsigned mFullMapIconOffset;
    unsigned mTeleportIconOffset;
    unsigned mEncampIconOffset;
    unsigned mCastIconOffset;
    std::vector<glm::vec2> mInventoryIconsDims;

    const Logging::Logger& mLogger;
};

}
