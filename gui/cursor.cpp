#include "gui/cursor.hpp"

#include "bak/textureFactory.hpp"

#include "com/assert.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/core/widget.hpp"

#include <stack>

namespace Gui {

Cursor::Cursor(
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

void Cursor::Clear()
{
    while (mCursors.size() > 1)
        mCursors.pop();
}

void Cursor::PushCursor(unsigned cursor)
{
    ASSERT(cursor < mSprites.size());
    mCursors.push(
        std::make_pair(
            mSprites.GetDimensions(cursor),
            cursor));
    mLogger.Spam() << "Pushed Cursor: " << cursor << "\n";
    UpdateCursor();
}

void Cursor::PopCursor()
{
    if (mCursors.size() < 2)
    {
        // This probably isn't an error...
        mLogger.Error() << "Only have one cursor, not popping it!\n";
    }
    else
    {
        mLogger.Spam() << "Popped Cursor: " << std::get<unsigned>(mCursors.top()) << "\n";
        mCursors.pop();
        UpdateCursor();
    }
}

void Cursor::Hide()
{
    SetColorMode(Graphics::ColorMode::SolidColor);
}

void Cursor::Show()
{
    SetColorMode(Graphics::ColorMode::Texture);
}

const Cursor::CursorInfo& Cursor::GetCursor()
{
    ASSERT(mCursors.size() >= 1);
    return mCursors.top();
}

const Graphics::Sprites& Cursor::GetSprites()
{
    return mSprites;
}

void Cursor::UpdateCursor()
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
    mLogger.Spam() << " Stack: " << ss.str() << std::endl;
    while (!cursors.empty())
    {
        auto tmp = cursors.top();
        cursors.pop();
        mCursors.push(tmp);
    }
}

}
