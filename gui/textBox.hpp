#pragma once

#include "bak/font.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/glm.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/fontManager.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

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
            //glm::vec4{0,1,0,.3},
            true
        },
        mText{}
    {
        // no point propagating MouseMoved to every
        // character of text
        SetInactive();
    }

    struct Line
    {
        std::vector<std::size_t> mChars;
        glm::vec2 mDimensions;
    };

    std::pair<glm::vec2, std::string_view> SetText(
        const Font& fr,
        std::string_view text,
        bool centerHorizontal=false,
        bool centerVertical=false,
        bool isBold=false,
        double newLineMultiplier=1.0,
        float scale=1.0)
    {
        const auto& logger = Logging::LogState::GetLogger("Gui::TextBox");

        ClearChildren();

        mText.clear();
        mText.reserve(text.size() * 2);

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
        auto moredhel = false;

        const auto NextLine = [&](bool halfLine){
            // Save this line's dims and move on to the next
            ASSERT(lines.size() > 0);
            auto ydiff = ((font.GetHeight() * newLineMultiplier + 1) / scale) * (halfLine ? .45 : 1.0);
            lines.back().mDimensions = glm::vec2{
                charPos.x + (font.GetSpace() / scale),
                charPos.y + ydiff
            };
            logger.Spam() << "NextLine: pos: " << charPos << " prevDims: " << lines.back().mDimensions << "\n";
            lines.emplace_back(Line{{}, glm::vec2{0}});

            charPos.x = initialPosition.x;
            charPos.y += ydiff;

            italic = false;
            unbold = false;
            red = false;
            white = false;
            emphasis = false;
            inWord = false;
        };

        const auto AdvanceChar = [&](auto w){
            charPos.x += (w / scale);
        };

        const auto Advance = [&](auto w){
            AdvanceChar(w);
            if (charPos.x > limit.x)
                NextLine(false);
        };

        unsigned wordLetters = 0;

        const auto Draw = [&](const auto& pos, auto c, const auto& color)
        {
            ASSERT(lines.size() > 0);
            lines.back().mChars.emplace_back(mText.size());
            mText.emplace_back(
                Graphics::DrawMode::Sprite,
                fr.GetSpriteSheet(),
                static_cast<Graphics::TextureIndex>(
                    font.GetIndex(c)),
                Graphics::ColorMode::ReplaceColor,
                color,
                pos,
                glm::vec2{fr.GetFont().GetWidth(c), fr.GetFont().GetHeight()} / scale,
                true);
        };

        const auto DrawNormal = [&](const auto& pos, auto c)
        {
            Draw(
                charPos,
                c,
                Color::black);
        };

        const auto DrawBold = [&](const auto& pos, auto c, auto bg, auto fg)
        {
            Draw(
                charPos + glm::vec2{0, 1},
                c,
                bg);

            Draw(
                charPos,
                c,
                fg);
        };

        const auto DrawMoredhel = [&](const auto& pos, auto c)
        {
            Draw(
                charPos + glm::vec2{0, -1},
                c,
                Color::moredhelFontUpper);

            Draw(
                charPos + glm::vec2{0, 1},
                c,
                Color::moredhelFontLower);

            Draw(
                charPos,
                c,
                Color::black);
        };
        
        unsigned currentChar = 0;
        for (; currentChar < text.size(); currentChar++)
        {
            if (text.size() == 0)
            {
                break;
            }
            const auto c = text[currentChar];
            logger.Spam() << "Char[" << c << "]" << std::hex 
                << +c << std::dec << " " << charPos << "\n";

            if (c == '\n')
            {
                NextLine(false);
            }
            else if (c == '\t')
            {
                Advance(font.GetSpace() * 4);
                bold = false;
            }
            else if (c == ' ')
            {
                if (moredhel)
                {
                    // moredhel text is very spaced...
                    Advance(font.GetSpace() * 6);
                }
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
            else if (c == static_cast<char>(0xf7))
            {
                moredhel = !moredhel;
            }
            else if (c == static_cast<char>(0xf8))
            {
                NextLine(true);
            }
            else if (c == static_cast<char>(0xe1)
                || c == static_cast<char>(0xe2) // not sure on e2
                || c == static_cast<char>(0xe3)) // not sure on e3
            {
                // Book text.. quoted or something
                DrawNormal(charPos, ' ');
                Advance(font.GetSpace() / 2.0);
            }
            else
            {
                if (moredhel)
                {
                    DrawMoredhel(charPos, c);
                }
                else if (bold)
                {
                    if (isBold)
                        DrawNormal(charPos, c);
                    else
                        DrawBold(charPos, c, Color::buttonShadow, Color::fontHighlight);
                }
                else if (unbold)
                {
                    // Maybe "lowlight", inactive
                    DrawBold(charPos, c, Color::black, Color::fontUnbold);
                    //DrawUnbold(charPos, c);
                }
                else if (red)
                {
                    DrawBold(charPos, c, Color::fontRedLowlight, Color::fontRedHighlight);
                }
                else if (white)
                {
                    DrawBold(charPos, c, Color::black, Color::fontWhiteHighlight);
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
                        DrawBold(charPos, c, Color::buttonShadow, Color::fontHighlight);
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
                    const auto saved = charPos;
                    const auto wordStart = std::next(text.begin(), nextChar);
                    const auto it = std::find_if(
                        wordStart,
                        std::next(text.begin(), text.size()),
                        [](const auto& c){ return c < '!' || c > 'z'; });

                    // Check if this word would overflow our bounds
                    wordLetters = std::distance(wordStart, it);
                    for (const auto& ch : text.substr(nextChar, wordLetters))
                        AdvanceChar(font.GetWidth(ch));
                    logger.Spam() << "Next Word: " << text.substr(nextChar, wordLetters) << "\n";

                    if (charPos.x >= limit.x)
                    {
                        charPos = saved;
                        NextLine(false);
                    }
                    else
                        charPos = saved;
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
                break;
        }

        // Set the dims of the final line
        logger.Spam() << "LastLine\n";
        NextLine(false);

        for (auto& elem : mText)
            this->AddChildBack(&elem);

        if (centerVertical)
        {
            const auto verticalAdjustment = limit.y > charPos.y
                ? (limit.y - charPos.y ) / 2.0
                : 0;

            for (auto& w : mText)
                w.AdjustPosition(
                    glm::vec2{0, verticalAdjustment});
            charPos.y += verticalAdjustment;
        }

        if (centerHorizontal)
        {
            for (const auto& line : lines)
            {
                const auto lineWidth = line.mDimensions.x;
                auto horizontalAdjustment = (limit.x - lineWidth) / 2.0;
                if (horizontalAdjustment < 0) horizontalAdjustment = 0;
                logger.Spam() << "Line: " << lineWidth << " lim: " << limit.x << " adj: " << horizontalAdjustment << "\n";
                for (const auto c : line.mChars)
                {
                    ASSERT(c < mText.size());
                    mText[c].AdjustPosition(
                        glm::vec2{horizontalAdjustment, 0});
                }
            }
        }

        auto maxX = std::max_element(
            lines.begin(), lines.end(),
            [](auto lhs, auto rhs){
                return lhs.mDimensions.x < rhs.mDimensions.x;
            });

        ASSERT(currentChar <= text.size() * 2);

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
