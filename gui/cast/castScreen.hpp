#pragma once

#include "bak/fileBufferFactory.hpp"
#include "bak/layout.hpp"
#include "bak/dialog.hpp"

#include "graphics/texture.hpp"

#include "gui/cast/symbol.hpp"

#include "gui/animator.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/line.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui::Cast {

class CastScreen : public Widget, public NullDialogScene
{
    enum class State
    {
        Idle,
        SpellSelected,
    };
    static constexpr auto sLayoutFile = "REQ_CAST.DAT";
    static constexpr auto sCombatLayoutFile = "SPELL.DAT";
    static constexpr auto sCastPanel = "CAST.SCX";
    static constexpr auto sScreen = "CFRAME.SCX";

    static constexpr auto sSymbol6 = 0;
    static constexpr auto sSymbol5 = 1;
    static constexpr auto sSymbol3 = 2;
    static constexpr auto sSymbol4 = 3;
    static constexpr auto sExit = 6;

    static constexpr auto sCharacterWidgetBegin = 7;

    static constexpr auto sSymbolTransitionTimeSeconds = .5;

public:
    CastScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        const Font& spellFont,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sCastPanel),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mGameState{gameState},
        mGuiManager{guiManager},
        mFont{font},
        mIcons{icons},
        mLayout{sLayoutFile},
        mCombatLayout{sCombatLayoutFile},
        mState{State::Idle},
        mInCombat{true},
        mSymbol{
            spellFont,
            mGameState,
            [&](auto spell){ HandleSpellClicked(spell); },
            [&](auto spell, bool selected){ HandleSpellHighlighted(spell, selected); }},
        mLines{},
        mButtons{},
        mSpellDesc{
            glm::vec2{134, 18},
            glm::vec2{176, 88}},
        mLogger{Logging::LogState::GetLogger("Gui::Cast")}
    {
        for (unsigned i = 0; i < 6; i++)
        {
            mLines.emplace_back(
                glm::vec2{72, 62.5},
                glm::vec2{72, 62.5},
                .5, glm::vec4{1,0,0,1});
        }

        /*
        for (unsigned i = 0; i < mCastData.GetClockTicks().size(); i++)
        {
            auto dot = mDots.emplace_back(
                [this, i=i]{ HandleDotClicked(i); },
                mIcons,
                mCastData.GetClockTicks().at(i),
                [](bool selected){});
            dot.SetCurrent(false);
        }
        */
        AddChildren();
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        bool handled = false;
        handled |= mSymbol.OnMouseEvent(event);
        for (auto& widget : mButtons)
        {
            handled |= widget.OnMouseEvent(event);
        }

        return handled;
    }

    void BeginCast(bool inCombat)
    {
        mInCombat = inCombat;
        assert(mGameState.GetParty().GetSpellcaster());
        mSelectedCharacter = *mGameState.GetParty().GetSpellcaster();
        mSymbol.SetActiveCharacter(mSelectedCharacter);
        PrepareLayout();
        AddChildren();

        if (mSymbol.GetSymbolIndex() == 0)
        {
            ChangeSymbol(mInCombat ? 3 : 5);
        }
    }

private:
    void PrepareLayout()
    {
        const auto& layout = mInCombat ? mCombatLayout : mLayout;
        mButtons.clear();
        mButtons.reserve(layout.GetSize());

        auto& party = mGameState.GetParty();
        BAK::ActiveCharIndex person{0};
        do
        {
            const auto [spriteSheet, image, dimss] = mIcons.GetCharacterHead(
                party.GetCharacter(person).GetIndex().mValue);
            mButtons.emplace_back(
                mLayout.GetWidgetLocation(person.mValue + sCharacterWidgetBegin),
                mLayout.GetWidgetDimensions(person.mValue + sCharacterWidgetBegin),
                spriteSheet,
                image,
                image,
                [this, character=person]{
                    SetActiveCharacter(character);
                },
                [this, character=person]{
                    SetActiveCharacter(character);
                }
            );
            
            person = party.NextActiveCharacter(person);
        } while (person != BAK::ActiveCharIndex{0});

        for (unsigned i = 0; i < layout.GetSize(); i++)
        {
            const auto& w = layout.GetWidget(i);
            if (w.mWidget == 0)
            {
                continue;
            }
            auto image = w.mImage;
            if ((w.mImage < 35 || w.mImage > 39) && !(w.mImage == 55 || w.mImage == 56) && (w.mImage != 13))
            {
                image = 25;
            }

            mButtons.emplace_back(
                layout.GetWidgetLocation(i),
                layout.GetWidgetDimensions(i) + (mInCombat ? glm::vec2{2, 3} : glm::vec2{}),
                std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(image)),
                std::get<Graphics::TextureIndex>(mIcons.GetButton(image)),
                std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(image)),
                [this, i]{ HandleButton(i); },
                []{});
        }
    }

    void SetActiveCharacter(BAK::ActiveCharIndex character)
    {
        if (!mGameState.GetParty().GetCharacter(character).IsSpellcaster())
        {
            mGuiManager.StartDialog(BAK::DialogSources::mCharacterIsNotASpellcaster, false, false, this);
        }
        else
        {
            mSelectedCharacter = character;
            mSymbol.SetActiveCharacter(character);
            mSymbol.SetSymbol(mSymbol.GetSymbolIndex());
        }
    }

    void HandleSpellClicked(BAK::SpellIndex spellIndex)
    {
        Logging::LogDebug(__FUNCTION__) << "(" << spellIndex << ")\n";
        mGameState.CastStaticSpell(BAK::ToStaticSpell(spellIndex), BAK::Times::TwelveHours);
    }

    void HandleSpellHighlighted(BAK::SpellIndex spellIndex, bool selected)
    {
        Logging::LogDebug(__FUNCTION__) << "(" << spellIndex << ", " << selected << ")\n";
        const auto& db = BAK::SpellDatabase::Get();
        std::stringstream ss{};
        if (selected)
        {
            const auto& doc = db.GetSpellDoc(spellIndex);
            ss << doc.mTitle << "\n";
            ss << "Cost: " << doc.mCost << "\n";
            if (!doc.mDamage.empty()) ss << doc.mDamage << "\n";
            if (!doc.mDuration.empty()) ss << doc.mDuration<< "\n";
            //ss << "Line Of Sight: " << doc.mLineOfSight<< "\n";
            ss << doc.mDescription << "\n";
        }
        else
        {
            for (const auto& spell : mSymbol.GetSpells())
            {
                if (mGameState.CanCastSpell(spell.mSpell, mSelectedCharacter))
                {
                    ss << db.GetSpellName(spell.mSpell) << "\n";
                }
            }
        }
        mSpellDesc.SetText(mFont, ss.str(), true, true);
        AddChildren();
    }

    void Exit()
    {
        // exit lock happens to do exactly what I want.. should probably rename it
        mGuiManager.DoFade(.8, [this]{mGuiManager.ExitLock(); });
    }

    void HandleButton(unsigned i)
    {
        if (i == sExit)
        {
            Exit();
        }
        else if (i == sSymbol5)
        {
            if (mInCombat)
                ChangeSymbol(3);
            else
                ChangeSymbol(5);
        }
        else if (i == sSymbol6)
        {
            if (mInCombat)
                ChangeSymbol(1);
            else
                ChangeSymbol(6);
        }
        else if (i == sSymbol3 && mInCombat)
        {
            ChangeSymbol(2);
        }
        else if (i == sSymbol4 && mInCombat)
        {
            ChangeSymbol(4);
        }

        AddChildren();
    }
    
    void ChangeSymbol(unsigned newSymbol)
    {
        mSymbol.Hide();

        const auto& points = BAK::SymbolLines::GetPoints(newSymbol - 1);
        for (unsigned i = 0; i < 6; i++)
        {
            const auto start = mLines[i].GetStart();
            const auto end = mLines[i].GetEnd();
            const auto startF = points[i];
            const auto endF = points[(i + 1) % 6];
            mGuiManager.AddAnimator(std::make_unique<LinearAnimator>(
                .5,
                glm::vec4{start.x, start.y, end.x, end.y},
                glm::vec4{startF.x, startF.y, endF.x, endF.y},
                [this, i](const auto& delta){
                    auto start = mLines[i].GetStart();
                    auto end = mLines[i].GetEnd();
                    mLines[i].SetPoints(
                        start + glm::vec2{delta.x, delta.y},
                        end + glm::vec2{delta.z, delta.w});
                    return false;
                },
                [this, startF, endF, i, newSymbol](){
                    mLines[i].SetPoints(startF, endF);
                    mSymbol.SetSymbol(newSymbol);
                    HandleSpellHighlighted(BAK::SpellIndex{}, false);
                }));
        }
    }

    void AddChildren()
    {
        ClearChildren();
        for (auto& widget : mButtons)
        {
            AddChildBack(&widget);
        }

        AddChildBack(&mSymbol);
        AddChildBack(&mSpellDesc);
        for (auto& line : mLines)
        {
            AddChildBack(&line);
        }
    }

    BAK::GameState& mGameState;

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::Layout mLayout;
    BAK::Layout mCombatLayout;

    State mState;
    bool mInCombat;
    BAK::ActiveCharIndex mSelectedCharacter;

    Symbol mSymbol;

    std::vector<Line> mLines;
    std::vector<ClickButtonImage> mButtons;
    TextBox mSpellDesc;

    const Logging::Logger& mLogger;
};

}
