#pragma once

#include "bak/textureFactory.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/widget.hpp"

#include <cassert>
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
            glm::vec4{1},
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

    void PushCursor(unsigned cursor)
    {
        assert(cursor < mSprites.size());
        mCursors.push(
            std::make_pair(
                mSprites.GetDimensions(cursor),
                cursor));
        mLogger.Spam() << "Pushed Cursor: " << cursor << "\n";
        UpdateCursor();
    }

    void PopCursor()
    {
        // We should always have at least one cursor
        assert(mCursors.size() >= 2);
        mLogger.Spam() << "Popped Cursor: " << std::get<unsigned>(mCursors.top()) << "\n";
        mCursors.pop();
        UpdateCursor();
    }

    const auto& GetCursor()
    {
        assert(mCursors.size() >= 1);
        return mCursors.top();
    }

    const Graphics::Sprites& GetSprites()
    {
        return mSprites;
    }

private:
    void UpdateCursor()
    {
        assert(mCursors.size() >= 1);
        const auto & [dimensions, texture] = GetCursor();
        mDrawInfo.mTexture = texture;
        mPositionInfo.mDimensions = dimensions;
    }

    std::stack<std::pair<Dimensions, CursorIndex>> mCursors;
    Graphics::Sprites& mSprites;
    const Logging::Logger& mLogger;
};

}
