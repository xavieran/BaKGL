#include "game/textAnimator.hpp"

#include "game/systems.hpp"
#include "game/glyphStore.hpp"

#include "bak/camera.hpp"
#include "bak/constants.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <algorithm>

namespace Game {

TextAnimator::TextAnimator(
    Systems& systems,
    const GlyphStore& glyphStore,
    const Camera& camera,
    glm::vec3 worldPos,
    std::string text,
    TextColor color,
    float lifetime)
:
    mSystems{systems},
    mWorldPosition{worldPos},
    mText{std::move(text)},
    mColor{color},
    mLifetime{lifetime},
    mElapsed{0.0f}
{
    auto col = mColor.mStart;

    glm::vec3 worldPosNorm = mWorldPosition / BAK::gWorldScale;
    glm::vec3 camPos = camera.GetNormalisedPosition();
    float distance = glm::distance(camPos, worldPosNorm);
    float scale = distance * sDamageTextScaleConstant;

    struct CharLayout
    {
        float mSpacing;
        float mGlyphScale;
        unsigned mOffset;
        unsigned mLength;
    };
    std::vector<CharLayout> layout;
    layout.reserve(mText.size());

    for (char c : mText)
    {
        auto* glyph = glyphStore.GetGlyph(c);
        if (glyph)
        {
            float glyphWidth = glyph->mGlyphAspect * scale;
            layout.push_back({
                glyphWidth + sGlyphGap * scale,
                glyphWidth,
                glyph->mOffset,
                glyph->mLength
            });
        }
    }

    float totalWorldWidth = 0;
    for (const auto& ci : layout)
        totalWorldWidth += ci.mSpacing;

    float cumulativeX = -totalWorldWidth / 2.0f;

    glm::vec3 toCamera = glm::normalize(camPos - worldPosNorm);
    glm::vec3 up = camera.GetUp();
    glm::vec3 layoutDir = glm::cross(up, toCamera);

    glm::vec3 billboardCenter = mWorldPosition / BAK::gWorldScale;

    mChars.reserve(layout.size());
    for (const auto& ci : layout)
    {
        float chPosX = cumulativeX;

        glm::vec3 location = mWorldPosition
            + layoutDir * (chPosX * BAK::gWorldScale);

        glm::mat4 modelMatrix = Graphics::CalculateModelMatrix(
            location,
            glm::vec3{ci.mGlyphScale, scale * sDamageTextGlyphStretch, 1.0f},
            glm::vec3{0},
            BAK::gWorldScale);

        if (ci.mLength > 0)
        {
            Graphics::TextRenderable tex{
                {ci.mOffset, ci.mLength},
                modelMatrix,
                col,
                billboardCenter};
            auto id = mSystems.AddTextRenderable(std::move(tex));
            mChars.push_back({id});
        }

        cumulativeX += ci.mSpacing;
    }
}

TextAnimator::~TextAnimator()
{
    for (const auto& ch : mChars)
    {
        mSystems.RemoveTextRenderable(ch.mSystemId);
    }
}

void TextAnimator::OnTimeDelta(double dt)
{
    mElapsed += static_cast<float>(dt);

    float t = std::min(mElapsed / mLifetime, 1.0f);
    auto color = glm::mix(mColor.mStart, mColor.mEnd, t * t);

    for (const auto& ch : mChars)
    {
        mSystems.GetTextRenderable(ch.mSystemId).mColor = color;
    }
}

bool TextAnimator::IsAlive() const
{
    return mElapsed < mLifetime;
}

}
