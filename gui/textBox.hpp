#pragma once

#include "bak/font.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/glm.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/fixedGuiElement.hpp"

#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

namespace Gui {

class FontRenderer
{
public:
    FontRenderer(
        const std::string& font,
        Graphics::SpriteManager& spriteManager)
    :
        mFont{std::invoke([&]{
            auto fb = FileBufferFactory::CreateFileBuffer(font);
            return BAK::LoadFont(fb);
        })},
        mSpriteSheet{std::invoke([&]{
            const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
            sprites.LoadTexturesGL(mFont.GetCharacters());
            return sheetIndex;
        })}
    {}

    const auto& GetFont() const { return mFont; }
    const auto& GetSpriteSheet() const { return mSpriteSheet; }

private:
    BAK::Font mFont;
    Graphics::SpriteSheetIndex mSpriteSheet;
};


class TextBox : public FixedGuiElement
{
public:
    TextBox(
        glm::vec3 pos,
        glm::vec3 dim)
    :
        FixedGuiElement{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            //glm::vec4{.2,.2,.2,.3},
            pos,
            dim,
            true
        }
    {
    }

    void AddText(
        const FontRenderer& fr,
        std::string_view text)
    {
        mText.clear();

        const auto& font = fr.GetFont();
        const auto initialPosition = glm::vec3{0, 0, 0};
        auto charPos = initialPosition;
        auto limit = initialPosition + GetPositionInfo().mDimensions;

        const auto NextLine = [&]{
            charPos.x = initialPosition.x;
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

        const auto Draw = [&](const auto& pos, auto c, const auto& color)
        {
            mText.emplace_back(
                Graphics::DrawMode::Sprite,
                fr.GetSpriteSheet(),
                static_cast<Graphics::TextureIndex>(c),
                Graphics::ColorMode::ReplaceColor,
                color,
                pos,
                glm::vec3{1},
                true);
        };

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
                Advance(font.GetSpace() * 4);
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
            else if (c == static_cast<char>(0xf1))
            {
            }
            else if (c == static_cast<char>(0xf3))
            {
            }
            else
            {
                Draw(
                    charPos,
                    font.GetIndex(c),
                    Color::black);

                if (bold)
                {
                    Draw(
                        charPos + glm::vec3{0,1,0},
                        font.GetIndex(c),
                        Color::buttonShadow);

                    Draw(
                        charPos,
                        font.GetIndex(c),
                        Color::fontHighlight);
                }

                Advance(font.GetWidth(c));

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
                break;
            }
        }

        for (auto& elem : mText)
            this->AddChildBack(&elem);

        return;
    }

private:
    std::vector<FixedGuiElement> mText;
};

}
