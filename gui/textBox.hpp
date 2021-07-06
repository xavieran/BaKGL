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
    std::optional<std::string_view> Render(
        const FontRenderer& fr,
        std::string_view text,
        DrawF&& Draw) const
    {
        const auto& font = fr.GetFont();
        auto charPos = mPosition;
        auto limit = mPosition + mDims;

        const auto NextLine = [&]{
            charPos.x = mPosition.x;
            charPos.y += font.GetHeight();
        };

        const auto AdvanceChar = [&](auto w){
            charPos.x += w;
        };

        const auto Advance = [&](auto w){
            AdvanceChar(w);
            if (charPos.x > limit.x)
                NextLine();
        };

        auto bold = false;
        auto inWord = false;
        unsigned wordLetters = 0;

        for (unsigned i = 0; i < text.size(); i++)
        {
            const auto c = text[i];

            if (c == '\n')
            {
                NextLine();
            }
            else if (c == '\t')
            {
                NextLine();
                Advance(font.GetSpace() * 5);
            }
            else if (c == ' ')
            {
                //|| c < font.GetFirstChar())
                Advance(font.GetSpace());
            }
            else if (c == '#')
            {
                bold = !bold;
            }
            else
            {
                auto textTrans = glm::translate(
                    glm::mat4{1},
                    charPos);

                Draw(
                    textTrans,
                    fr.GetSprites().Get(
                        font.GetIndex(c)));
                std::cout << c;
                Advance(font.GetWidth(c));

                if (bold)
                {
                    float boldness = 8.8f / 8.0f;

                    textTrans = glm::translate(
                        textTrans,
                        -glm::vec3{0.6f, 0.6f, 0});

                    textTrans = glm::scale(
                        textTrans,
                        glm::vec3{boldness, boldness, 0.0f});

                    Draw(
                        textTrans,
                        fr.GetSprites().Get(
                            font.GetIndex(c)));
                }
            }
            const auto nextChar = i + 1;
            if (nextChar < text.size())
            {
                const auto ch = text[nextChar];
                const auto isAlphaNum = ch >= '!' || c <= 'z';

                if (isAlphaNum && !inWord)
                {
                    const auto tmpPos = charPos;
                    const auto wordStart = text.begin() + nextChar;
                    const auto it = std::find_if(
                        wordStart,
                        text.begin() + text.size(),
                        [](const auto& c){ return c < '!' || c > 'z'; });

                    wordLetters = std::distance(wordStart, it);
                    for (const auto& ch : text.substr(nextChar, wordLetters))
                        AdvanceChar(font.GetWidth(ch));

                    if (charPos.x >= limit.x)
                        NextLine();
                    else
                        charPos = tmpPos;
                }
                else if (isAlphaNum)
                    inWord = true;
                else
                    inWord = false;
            }

            if (charPos.y > limit.y)
            {
                return text.substr(i, text.size() - 1);
            }
        }

        return std::optional<std::string_view>{};
    }

private:
    glm::vec3 mPosition;
    glm::vec3 mDims;
};

}
