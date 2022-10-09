#pragma once

#include "bak/textureFactory.hpp"

#include "com/assert.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/widget.hpp"

#include <stack>

namespace Gui {

class Cursor : public Widget
{
public:
    using Dimensions = glm::vec2;
    using CursorIndex = unsigned;

    Cursor(
        Graphics::SpriteManager& spriteManager)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            spriteManager.AddSpriteSheet(),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{0},
            glm::vec2{1},
            false
        },
        mCursors{},
        mSprites{
            spriteManager.GetSpriteSheet(
                mDrawInfo.mSpriteSheet)},
        mLogger{Logging::LogState::GetLogger("Gui::Cursor")}
    {
        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddToTextureStore(
            textures,
            "POINTERG.BMX",
            "OPTIONS.PAL");
        mSprites.LoadTexturesGL(textures);

        PushCursor(0);
    }

    void Clear()
    {
        while (mCursors.size() > 1)
            mCursors.pop();
    }

    void PushCursor(unsigned cursor)
    {
        ASSERT(cursor < mSprites.size());
        mCursors.push(
            std::make_pair(
                mSprites.GetDimensions(cursor),
                cursor));
        mLogger.Info() << "Pushed Cursor: " << cursor << "\n";
        UpdateCursor();
    }

    void PopCursor()
    {
        // We should always have at least one cursor
        ASSERT(mCursors.size() >= 2);
        mLogger.Info() << "Popped Cursor: " << std::get<unsigned>(mCursors.top()) << "\n";
        mCursors.pop();
        UpdateCursor();
    }

    void Hide()
    {
        SetColorMode(Graphics::ColorMode::SolidColor);
    }

    void Show()
    {
        SetColorMode(Graphics::ColorMode::Texture);
    }

    const auto& GetCursor()
    {
        ASSERT(mCursors.size() >= 1);
        return mCursors.top();
    }

    const Graphics::Sprites& GetSprites()
    {
        return mSprites;
    }

private:
    void UpdateCursor()
    {
        ASSERT(mCursors.size() >= 1);
        const auto & [dimensions, texture] = GetCursor();
        mDrawInfo.mTexture = Graphics::TextureIndex{texture};
        mPositionInfo.mDimensions = dimensions;

        std::stringstream ss{};
        std::stack<std::pair<Dimensions, CursorIndex>> cursors{};
        while (!mCursors.empty())
        {
            auto tmp = mCursors.top();
            mCursors.pop();
            cursors.push(tmp);
            ss << " " << tmp.second << ",";
        }
        mLogger.Info() << " Stack: " << ss.str() << std::endl;
        while (!cursors.empty())
        {
            auto tmp = cursors.top();
            cursors.pop();
            mCursors.push(tmp);
        }
    }

    std::stack<std::pair<Dimensions, CursorIndex>> mCursors;
    Graphics::Sprites& mSprites;
    const Logging::Logger& mLogger;
};

}
