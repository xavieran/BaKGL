#pragma once

#include "bak/condition.hpp"
#include "bak/skills.hpp"

#include "gui/actors.hpp"
#include "gui/colors.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class Ratings : public Widget
{
public:
    Ratings(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex verticalBorder,
        Graphics::TextureIndex horizontalBorder)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            Color::infoBackground,
            true
        },
        mFont{font},
        mClipRegion{
            ClipRegionTag{},
            glm::vec2{0},
            dims,
            true},
        mRatings{
            RectTag{},
            glm::vec2{0},
            dims,
            Color::infoBackground,
            true},
        mRatingsText{
            glm::vec2{10, 8},
            dims
        },
        mConditionsText{
            glm::vec2{120, 8},
            dims
        },
        mLeftBorder{
            ImageTag{},
            spriteSheet,
            verticalBorder,
            glm::vec2{0},
            glm::vec2{2, 72},
            true},
        mRightBorder{
            ImageTag{},
            spriteSheet,
            verticalBorder,
            glm::vec2{220, 0},
            glm::vec2{2, 72},
            true},
        mTopBorder{
            ImageTag{},
            spriteSheet,
            horizontalBorder,
            glm::vec2{2, 0},
            glm::vec2{222, 2},
            true},
        mBottomBorder{
            ImageTag{},
            spriteSheet,
            horizontalBorder,
            glm::vec2{1, 71},
            glm::vec2{222, 2},
            true},
        mLogger{Logging::LogState::GetLogger("Gui::Ratings")}
    {
        AddChildBack(&mClipRegion);
        mClipRegion.AddChildBack(&mRatings);
        mClipRegion.AddChildBack(&mRatingsText);
        mClipRegion.AddChildBack(&mConditionsText);
        mClipRegion.AddChildBack(&mLeftBorder);
        mClipRegion.AddChildBack(&mRightBorder);
        mClipRegion.AddChildBack(&mTopBorder);
        mClipRegion.AddChildBack(&mBottomBorder);
    }

    void SetCharacter(
        const BAK::Skills& skills,
        const BAK::Conditions& conditions)
    {
        {
            std::stringstream ss{};
            ss << "\xf6Ratings: \n";
            const auto& health = skills.GetSkill(BAK::SkillType::Health);
            if (health.mUnseenImprovement) ss << "\xf5";
            else ss << "\xf6";
            ss << "  Health      " << +health.mCurrent << " of " << +health.mMax << "\n";
            const auto& stamina = skills.GetSkill(BAK::SkillType::Stamina);
            if (stamina.mUnseenImprovement) ss << "\xf5";
            else ss << "\xf6";
            ss << "  Stamina    " << +stamina.mCurrent << " of " << +stamina.mMax << "\n";
            const auto& speed = skills.GetSkill(BAK::SkillType::Speed);
            if (speed.mUnseenImprovement) ss << "\xf5";
            else ss << "\xf6";
            ss << "  Speed       " << +speed.mCurrent << "\n";
            const auto& strength = skills.GetSkill(BAK::SkillType::Strength);
            if (strength.mUnseenImprovement) ss << "\xf5";
            else ss << "\xf6";
            ss << "  Strength    " << +strength.mCurrent << "\n";
            mLogger.Debug() << ss.str();
            mRatingsText.AddText(mFont, ss.str());
        }
        {
            std::stringstream ss{};
            ss << "\xf6" "Condition: \n";
            if (conditions.NoConditions())
            {
                ss << "\xf6  Normal";
            }
            else
            {
                for (unsigned i = 0; i < conditions.sNumConditions; i++)
                {
                    const auto type = static_cast<BAK::Condition>(i);
                    const auto value = conditions.GetCondition(type);
                    if (value != 0)
                    {
                        ss << "\xf5  " << BAK::ToString(type) << " (" << value << "%)\n";
                    }
                }
            }
            mConditionsText.AddText(mFont, ss.str());
        }
    }

private:
    const Font& mFont;

    Widget mClipRegion;

    Widget mRatings;
    TextBox mRatingsText;
    TextBox mConditionsText;

    Widget mLeftBorder;
    Widget mRightBorder;
    Widget mTopBorder;
    Widget mBottomBorder;

    const Logging::Logger& mLogger;
};

}
