#pragma once

#include "bak/font.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/glm.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/widget.hpp"

#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

namespace Gui {

class Font
{
public:
    Font(
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
            RectTag{},
            pos,
            dim,
            glm::vec4{0},
            true
        },
        mText{}
    {
        // should be enough...
        mText.reserve(2048);
        // no point propagating MouseMoved to every
        // character of text
        SetInactive();
    }

    struct Line
    {
        std::vector<Widget*> mChars;
        glm::vec2 mDimensions;
    };

    std::pair<glm::vec2, std::string_view> AddText(
        const Font& fr,
        std::string_view text,
        bool centerHorizontal=false,
        bool centerVertical=false,
        bool isBold=false)
    {
        // otherwise iters not stable...
        //ASSERT(text.size() < 2048);

        mText.clear();

        ClearChildren();

        const auto& font = fr.GetFont();
        const auto initialPosition = glm::vec2{0};
        auto charPos = initialPosition;
        auto limit = initialPosition + GetPositionInfo().mDimensions;

        std::vector<Line> lines{};
        lines.emplace_back(Line{{}, glm::vec2{0}});

        auto italic   = false;
        auto emphasis = false;
        auto bold     = false;
        auto unbold   = false;
        auto red      = false;
        auto white    = false;
        auto inWord   = false;

        const auto NextLine = [&]{
            // Save this line's dims and move on to the next
            ASSERT(lines.size() > 0);
            lines.back().mDimensions = glm::vec2{
                charPos.x + font.GetSpace(),
                charPos.y + font.GetHeight() + 1
            };
            lines.emplace_back(Line{{}, glm::vec2{0}});

            charPos.x = initialPosition.x;
            charPos.y += font.GetHeight() + 1;

            italic = false;
            unbold = false;
            red = false;
            white = false;
            emphasis = false;
            inWord = false;
        };

        const auto AdvanceChar = [&](auto w){
            charPos.x += w;
        };

        const auto Advance = [&](auto w){
            AdvanceChar(w);
            if (charPos.x > limit.x)
                NextLine();
        };

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

            ASSERT(lines.size() > 0);
            lines.back().mChars.emplace_back(&mText.back());
        };

        const auto DrawNormal = [&](const auto& pos, auto c)
        {
            Draw(
                charPos,
                c,
                Color::black);
        };

        const auto DrawBold = [&](const auto& pos, auto c)
        {
            Draw(
                charPos + glm::vec2{0, 1},
                c,
                Color::buttonShadow);

            Draw(
                charPos,
                c,
                Color::fontHighlight);
        };

        const auto DrawUnbold = [&](const auto& pos, auto c)
        {
            Draw(
                charPos + glm::vec2{0, 1},
                c,
                Color::black);

            Draw(
                charPos,
                c,
                Color::fontUnbold);
        };

        const auto DrawRed = [&](const auto& pos, auto c)
        {
            Draw(
                charPos + glm::vec2{1, 1},
                c,
                Color::fontRedLowlight);

            Draw(
                charPos,
                c,
                Color::fontRedHighlight);
        };

        const auto DrawWhite= [&](const auto& pos, auto c)
        {
            Draw(
                charPos + glm::vec2{1, 1},
                c,
                Color::black);

            Draw(
                charPos,
                c,
                Color::fontWhiteHighlight);
        };
        
        unsigned currentChar = 0;
        for (; currentChar < text.size(); currentChar++)
        {
            const auto c = text[currentChar];

            if (c == '\n')
            {
                NextLine();
            }
            else if (c == '\t')
            {
                Advance(font.GetSpace() * 4);
                bold = false;
            }
            else if (c == ' ')
            {
                Advance(font.GetSpace());
                emphasis = false;
                italic = false;
            }
            else if (c == '#')
            {
                bold = !bold;
            }
            else if (c == static_cast<char>(0xf4))
            {
                unbold = !unbold;
            }
            else if (c == static_cast<char>(0xf5))
            {
                red = !red;
            }
            else if (c == static_cast<char>(0xf6))
            {
                white = !white;
            }
            else if (c == static_cast<char>(0xf0))
            {
                emphasis = true;
            }
            else if (c == static_cast<char>(0xf1))
            {
                emphasis = true;
            }
            else if (c == static_cast<char>(0xf3))
            {
                italic = true;
            }
            else if (c == static_cast<char>(0xe1))
            {
                // Book text.. quoted or something
                DrawNormal(charPos, ' ');
                Advance(font.GetSpace() / 2.0);
            }
            else
            {
                if (bold)
                {
                    if (isBold)
                        DrawNormal(charPos, c);
                    else
                        DrawBold(charPos, c);
                }
                else if (unbold)
                {
                    // Maybe "lowlight", inactive
                    DrawUnbold(charPos, c);
                }
                else if (red)
                {
                    DrawRed(charPos, c);
                }
                else if (white)
                {
                    DrawWhite(charPos, c);
                }
                else if (emphasis)
                {
                    Draw(
                        charPos,
                        c,
                        Color::fontEmphasis);
                }
                else if (italic)
                {
                    Draw(
                        charPos,
                        c,
                        Color::fontLowlight);
                    // Draw italic...
                }
                else
                {
                    if (isBold)
                        DrawBold(charPos, c);
                    else
                        DrawNormal(charPos, c);
                }

                Advance(font.GetWidth(c));
            }

            const auto nextChar = currentChar + 1;
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

                    // Check if this word would overflow our bounds
                    wordLetters = std::distance(wordStart, it);
                    for (const auto& ch : text.substr(nextChar, wordLetters))
                        AdvanceChar(font.GetWidth(ch));

                    if (charPos.x >= limit.x)
                        NextLine();
                    else
                        charPos = tmpPos;
                }
                else if (isAlphaNum)
                {
                    inWord = true;
                }
                else
                {
                    // Exiting a word
                    emphasis = false;
                    italic = false;
                    inWord = false;
                }
            }

            if (charPos.y + font.GetHeight() > limit.y)
            {
                break;
            }
        }

        // Set the dims of the final line
        NextLine();

        for (auto& elem : mText)
            this->AddChildBack(&elem);

        if (centerVertical)
        {
            const auto verticalAdjustment = limit.y > charPos.y
                ? (limit.y - charPos.y ) / 2.0
                : 0;

            for (auto& w : mText)
            {
                w.AdjustPosition(
                    glm::vec2{0, verticalAdjustment});
            }
        }


        if (centerHorizontal)
        {
            for (const auto& line : lines)
            {
                const auto lineWidth = line.mDimensions.x;
                const auto horizontalAdjustment = (limit.x - lineWidth) / 2.0;
                for (auto* c : line.mChars)
                {
                    ASSERT(c);
                    c->AdjustPosition(
                        glm::vec2{horizontalAdjustment, 0});
                }
            }
        }

        auto maxX = std::max_element(
            lines.begin(), lines.end(), [](auto lhs, auto rhs)
            {
                return lhs.mDimensions.x < rhs.mDimensions.x;
            });

        return std::make_pair(
            glm::vec2{maxX->mDimensions.x, charPos.y},
            text.substr(
                currentChar,
                text.size() - currentChar));
    }

private:
    std::vector<Widget> mText;
};

}
