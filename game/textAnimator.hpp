#pragma once

#include "bak/types.hpp"

#include "game/glyphStore.hpp"

#include "gui/IAnimator.hpp"

#include "graphics/renderer.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

class Camera;
class Systems;

namespace Game {
class GlyphStore;

class TextAnimator : public Gui::IAnimator
{
public:
    TextAnimator(
        Systems& systems,
        const GlyphStore& glyphStore,
        const Camera& camera,
        glm::vec3 worldPos,
        std::string text,
        TextColor color,
        float duration);

    ~TextAnimator();

    void OnTimeDelta(double dt) override;
    bool IsAlive() const override;

private:
    Systems& mSystems;
    glm::vec3 mWorldPosition;
    std::string mText;
    TextColor mColor;
    float mDuration;

    struct CharEntry
    {
        BAK::EntityIndex mSystemId;
    };
    std::vector<CharEntry> mChars;

    float mElapsed{};
    bool mAlive{true};

    static constexpr float sDamageTextScaleConstant = 0.014f;
    static constexpr float sGlyphGap = 0.25f;
    static constexpr float sDamageTextGlyphStretch = 1.2f;
};

}
