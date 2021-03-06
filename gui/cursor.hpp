#pragma once

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include <cassert>

namespace Gui {

class Cursor
{
public:

    Cursor()
    :
        mCursors{{0}},
        mSprites{std::invoke([]{
            auto textures = Graphics::TextureStore{};
            BAK::TextureFactory::AddToTextureStore(
                textures,
                "POINTERG.BMX",
                "OPTIONS.PAL");

            auto sprites = Graphics::Sprites{};
            sprites.LoadTexturesGL(textures);
            return sprites;
        })},
        mLogger{Logging::LogState::GetLogger("Gui::Cursor")}
    {}

    unsigned PushCursor(unsigned cursor)
    {
        assert(cursor < mSprites.size());
        mCursors.push(cursor);
    }

    unsigned PopCursor()
    {
        // We should always have at least one cursor
        assert(mCursors.size() >= 2);
        mCursors.pop();
    }

    unsigned GetCursor()
    {
        assert(mCursors.size() >= 1);
        return mCursors.top();
    }

    const Graphics::Sprites& GetSprites()
    {
        return mSprites;
    }

private:
    std::stack<unsigned> mCursors;
    Graphics::Sprites mSprites;
    const Logging::Logger& mLogger;
};

}
