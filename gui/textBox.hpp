#pragma once

#include "bak/font.hpp"

#include "com/logger.hpp"

#include "graphics/IGuiElement.hpp"
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
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            //Color::debug,
            glm::vec4{0},
            pos,
            dim,
            true
        },
        mText{}
    {
        // should be enough...
        mText.reserve(2048);
    }

    struct Line
    {
        std::vector<Widget*> mChars;
        glm::vec2 mDimensions;
    };

    glm::vec2 AddText(
        const Font& fr,
        std::string_view text,
        bool centerHorizontal=false,
        bool centerVertical=false,
        bool isBold=false)
    {
        // otherwise iters not stable...
        assert(text.size() < 2048);

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
        auto inWord   = false;

        const auto NextLine = [&]{
            // Save this line's dims and move on to the next
            assert(lines.size() > 0);
            lines.back().mDimensions = glm::vec2{
                charPos.x + font.GetSpace(),
                charPos.y + font.GetHeight()
            };
            lines.emplace_back(Line{{}, glm::vec2{0}});

            charPos.x = initialPosition.x;
            charPos.y += font.GetHeight();

            italic = false;
            bold = false;
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

            assert(lines.size() > 0);
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

        for (unsigned i = 0; i < text.size(); i++)
        {
            const auto c = text[i];

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
            else
            {
                if (bold)
                {
                    if (isBold)
                        DrawNormal(charPos, c);
                    else
                        DrawBold(charPos, c);
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

            if (charPos.y > limit.y)
            {
                break;
            }
        }

        // Trigger the final line being set
        NextLine();

        for (auto& elem : mText)
            this->AddChildBack(&elem);

        // charPos is now the final dims of the drawn text
        charPos.x += font.GetSpace();
        charPos.y += font.GetHeight();

        if (centerVertical)
        {
            const auto verticalAdjustment = (limit.y - charPos.y) / 2.0;
            Logging::LogDebug("TEXT") << "Height: " << charPos.y << " limit: " 
                    << limit << " vertAdj: " << verticalAdjustment << "\n";
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
                Logging::LogDebug("TEXT") << "LW: " << lineWidth << " limit: " 
                    << limit << " horzAdj: " << horizontalAdjustment << "\n";
                for (auto* c : line.mChars)
                {
                    assert(c);
                    c->AdjustPosition(
                        glm::vec2{horizontalAdjustment, 0});
                }
            }
        }

        return charPos;
    }

private:
    std::vector<Widget> mText;
};

}
