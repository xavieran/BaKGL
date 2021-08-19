#pragma once

#include "bak/font.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/glm.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/widget.hpp"

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
        mSpriteSheet{spriteManager.AddSpriteSheet()}
    {
        spriteManager.GetSpriteSheet(mSpriteSheet)
            .LoadTexturesGL(mFont.GetCharacters());
    }

    const auto& GetFont() const { return mFont; }
    const auto& GetSpriteSheet() const { return mSpriteSheet; }

private:
    BAK::Font mFont;
    Graphics::SpriteSheetIndex mSpriteSheet;
};


class TextBox : public Widget
{
public:
    TextBox(
        glm::vec2 pos,
        glm::vec2 dim)
    :
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            pos,
            dim,
            true
        }
    {
    }

    glm::vec2 AddText(
        const FontRenderer& fr,
        std::string_view text)
    {
        mText.clear();
        ClearChildren();

        const auto& font = fr.GetFont();
        const auto initialPosition = glm::vec2{0};
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
                static_cast<Graphics::TextureIndex>(
                    font.GetIndex(c)),
                Graphics::ColorMode::ReplaceColor,
                color,
                pos,
                glm::vec2{fr.GetFont().GetWidth(c), fr.GetFont().GetHeight()},
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
                    c,
                    Color::black);

                if (bold)
                {
                    Draw(
                        charPos + glm::vec2{0,1},
                        c,
                        Color::buttonShadow);

                    Draw(
                        charPos,
                        c,
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

        // Bounding box...
        charPos.y += font.GetHeight();
        return charPos;
    }

private:
    std::vector<Widget> mText;
};

}
