#pragma once

#include "graphics/glm.hpp"
#include "graphics/sprites.hpp"

#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

namespace Gui {

class FontRenderer
{
public:
    FontRenderer(
        const std::string& font)
    :
        mFont{std::invoke([&]{
            auto fb = FileBufferFactory::CreateFileBuffer(font);
            return BAK::LoadFont(fb);
        })},
        mSprites{std::invoke([&]{
            auto sprites = Graphics::Sprites{};
            sprites.LoadTexturesGL(mFont.GetCharacters());
            return sprites;
        })}
    {}

    const auto& GetFont() const { return mFont; }
    const auto& GetSprites() const { return mSprites; }

private:
    BAK::Font mFont;
    Graphics::Sprites mSprites;
};

class TextBox
{
public:
    TextBox(
        glm::vec3 pos,
        glm::vec3 dim)
    :
        mPosition{pos},
        mDims{dim}
    {
    }

    template <typename DrawF>
    void Render(
        const FontRenderer& fr,
        std::string_view text,
        DrawF&& Draw)
    {
        const auto& font = fr.GetFont();
        auto charPos = mPosition;
        auto limit = mPosition + mDims;

        const auto NextLine = [&]{
            charPos.x = mPosition.x;
            charPos.y += font.GetHeight();
        };

        const auto Advance = [&](auto w){
            charPos.x += w;
            if (charPos.x > limit.x)
                NextLine();
        };

        auto bold = false;

        for (const char c : text)
        {
            if (c == '\n' || c == '\t')
                NextLine();
            else if (c == ' '
                || c < font.GetFirstChar())
                Advance(font.GetSpace());
            else if (c == '#')
                bold = !bold;
            else
            {
                auto textTrans = glm::translate(
                    glm::mat4{1},
                    charPos);

                Draw(
                    textTrans,
                    fr.GetSprites().Get(
                        font.GetIndex(c)));
                    Advance(font.GetWidth(c));

                if (bold)
                {
                    float boldness = 8.8f / 8.0f;

                    textTrans = glm::translate(
                        textTrans,
                        -glm::vec3{0.6f, 0.6f, 0});

                    textTrans = glm::scale(
                        textTrans,
                        glm::vec3{
                            boldness,
                            boldness,
                            0.0f});
                    Draw(
                        textTrans,
                        fr.GetSprites().Get(
                            font.GetIndex(c)));
                        Advance(2);
                }
            }
        }
    }

private:
    glm::vec3 mPosition;
    glm::vec3 mDims;
};

}
