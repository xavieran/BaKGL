#pragma once

#include "bak/fileBufferFactory.hpp"
#include "bak/layout.hpp"
#include "bak/dialog.hpp"
#include "bak/skills.hpp"

#include "graphics/texture.hpp"

#include "gui/cast/symbol.hpp"
#include "gui/cast/powerRing.hpp"

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
    static constexpr auto sSymbol2 = 2;
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
        mSelectedSpell{},
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
        mPowerRing{mIcons, glm::vec2{}, [&](unsigned power){ HandleSpellPower(power); }},
        mLogger{Logging::LogState::GetLogger("Gui::Cast")}
    {
        for (unsigned i = 0; i < 6; i++)
        {
            mLines.emplace_back(
                glm::vec2{72, 62.5},
                glm::vec2{72, 62.5},
                .5, glm::vec4{1,0,0,1});
        }

        AddChildren();
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        bool handled = false;
        if (mState == State::Idle)
        {
            handled |= mSymbol.OnMouseEvent(event);
            for (auto& widget : mButtons)
            {
                handled |= widget.OnMouseEvent(event);
            }
        }
        else
        {
            handled |= mPowerRing.OnMouseEvent(event);
        }

        return handled;
    }

    void BeginCast(bool inCombat)
    {
        mInCombat = inCombat;

        if (mSymbol.GetSymbolIndex() == 0)
        {
            ChangeSymbol(mInCombat ? 3 : 5);
        }

        assert(mGameState.GetParty().GetSpellcaster());
        SetActiveCharacter(*mGameState.GetParty().GetSpellcaster());
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

            if (mSelectedCharacter != person)
            {
                mButtons.back().SetColor(glm::vec4{.05, .05, .05, 1}); 
                mButtons.back().SetColorMode(Graphics::ColorMode::TintColor);
            }
            
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
            mGameState.SetActiveCharacter(mGameState.GetParty().GetCharacter(mSelectedCharacter).mCharacterIndex);
            mSymbol.SetActiveCharacter(character);
            if (mSymbol.GetSymbolIndex() > 0)
            {
                mSymbol.SetSymbol(mSymbol.GetSymbolIndex());
            }
            PrepareLayout();
            AddChildren();
        }
    }

    void HandleSpellClicked(BAK::SpellIndex spellIndex)
    {
        Logging::LogDebug(__FUNCTION__) << "(" << spellIndex << ")\n";
        const auto& spell = BAK::SpellDatabase::Get().GetSpell(spellIndex);
        auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
        mSelectedSpell = spellIndex;
        if (spell.mMinCost != spell.mMaxCost)
        {
            const auto health = character.GetSkill(BAK::SkillType::TotalHealth);
            mState = State::SpellSelected;
            mPowerRing.Animate(std::make_pair(spell.mMinCost, std::min(spell.mMaxCost, health)), mGuiManager);
            AddChildren();
        }
        else
        {
            CastSpell(0);
        }
    }

    void HandleSpellPower(unsigned power)
    {
        CastSpell(power);
        mState = State::Idle;
        HandleSpellHighlighted(BAK::SpellIndex{0}, false);
        AddChildren();
    }

    void CastSpell(unsigned power)
    {
        assert(mSelectedSpell);
        const auto spellIndex = *mSelectedSpell;
        mGameState.CastStaticSpell(BAK::ToStaticSpell(spellIndex), BAK::Times::OneHour * power);
        mSelectedSpell.reset();
        auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
        character.ImproveSkill(BAK::SkillType::TotalHealth, BAK::SkillChange::HealMultiplier_100, (-power) << 8);
        mGuiManager.StartDialog(
            BAK::DialogSources::GetSpellCastDialog(static_cast<unsigned>(BAK::ToStaticSpell(spellIndex))),
            false,
            false,
            this);
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
            if (!doc.mLineOfSight.empty()) ss << doc.mLineOfSight << "\n";
            ss << doc.mDescription << "\n";
            ss << "\n";

            const auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
            const auto health = character.GetSkill(BAK::SkillType::TotalHealth);
            const auto maxHealth = character.GetMaxSkill(BAK::SkillType::TotalHealth);
            ss << "Health/Stamina:  " << health << " of " << maxHealth << "\n";
        }
        else
        {
            if (mSymbol.GetSymbolIndex() > 0)
            {
                for (const auto& spell : mSymbol.GetSpells())
                {
                    if (mGameState.CanCastSpell(spell.mSpell, mSelectedCharacter))
                    {
                        ss << db.GetSpellName(spell.mSpell) << "\n";
                    }
                }
            }
        }
        mSpellDesc.SetText(mFont, ss.str(), true, true);
        AddChildren();
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override
    {
        Exit();
    }

    void Exit()
    {
        // exit lock happens to do exactly what I want.. should probably rename it
        mGuiManager.DoFade(.8, [this]{ mGuiManager.ExitLock(); });
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
        else if (i == sSymbol2 && mInCombat)
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
        if (newSymbol == mSymbol.GetSymbolIndex()) return;

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

        if (mState == State::SpellSelected)
        {
            AddChildBack(&mPowerRing);
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
    std::optional<BAK::SpellIndex> mSelectedSpell;
    BAK::ActiveCharIndex mSelectedCharacter;

    Symbol mSymbol;

    std::vector<Line> mLines;
    std::vector<ClickButtonImage> mButtons;
    TextBox mSpellDesc;
    PowerRing mPowerRing;

    const Logging::Logger& mLogger;
};

}
