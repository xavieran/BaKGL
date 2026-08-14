#include "game/textAnimator.hpp"

#include "game/systems.hpp"
#include "game/glyphStore.hpp"

#include "bak/constants.hpp"

#include <glm/glm.hpp>

#include <algorithm>

namespace Game {

TextAnimator::TextAnimator(
    Systems& systems,
    const GlyphStore& glyphStore,
    glm::vec3 worldPos,
    std::string text,
    TextColor color,
    float duration)
:
    mSystems{systems},
    mWorldPosition{worldPos},
    mText{std::move(text)},
    mColor{color},
    mDuration{duration}
{
    auto col = mColor.mStart;

    const float glyphHeight = sDamageTextScaleConstant * sDamageTextGlyphStretch;
    const float gap = sGlyphGap * sDamageTextScaleConstant;

    struct CharLayout
    {
        float mWidth;
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
            layout.push_back({
                glyph->mGlyphAspect * sDamageTextScaleConstant,
                glyph->mOffset,
                glyph->mLength
            });
        }
    }

    float totalWidth = 0;
    for (const auto& ci : layout)
        totalWidth += ci.mWidth + gap;
    if (!layout.empty())
        totalWidth -= gap;

    float cumulativeX = -totalWidth / 2.0f;

    glm::vec3 billboardCenter = mWorldPosition / BAK::gWorldScale;

    mChars.reserve(layout.size());
    for (const auto& ci : layout)
    {
        if (ci.mLength > 0)
        {
            Graphics::TextRenderable tex{
                {ci.mOffset, ci.mLength},
                billboardCenter,
                glm::vec2{cumulativeX + ci.mWidth / 2.0f, 0},
                glm::vec2{ci.mWidth, glyphHeight},
                col};
            auto id = mSystems.AddTextRenderable(std::move(tex));
            mChars.push_back({id});
        }

        cumulativeX += ci.mWidth + gap;
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

    if (mElapsed >= mDuration)
    {
        mAlive = false;
    }

    float t = std::min(mElapsed / mDuration, 1.0f);
    auto color = glm::mix(mColor.mStart, mColor.mEnd, t * t);

    for (const auto& ch : mChars)
    {
        auto* renderable = mSystems.GetTextRenderable(ch.mSystemId);
        if (renderable)
        {
            renderable->mColor = color;
        }
        else
        {
            mAlive = false;
        }
    }
}

bool TextAnimator::IsAlive() const
{
    return mAlive;
}

}
