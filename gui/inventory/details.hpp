#pragma once

#include "bak/dialogSources.hpp"
#include "bak/spells.hpp"
#include "bak/gameState.hpp"
#include "bak/inventoryItem.hpp"

#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/centeredImage.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>


namespace Gui {

class Details : public Widget
{
public:
    Details(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font,
        std::function<void()>&& finished)
    :
        Widget{
            RectTag{},
            glm::vec2{13, 11},
            glm::vec2{289, 121},
            Color::black,
            true
        },
        mIcons{icons},
        mFont{font},
        mName{
            glm::vec2{6, 3},
            glm::vec2{90, 40}
        },
        mItem{
            glm::vec2{8, 31},
            glm::vec2{80, 58}
        },
        mStatusText{
            glm::vec2{8, 27 + 58},
            glm::vec2{90, 22}
        },
        mDescriptionBackground{
            glm::vec2{98, 0},
            glm::vec2{196, 121},
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow
        },
        mDescriptionText{
            glm::vec2{104, 0},
            glm::vec2{182, 120}
        },
        mMoreInfo{
            glm::vec2{8, 5 + 28 + 58 + 12},
            glm::vec2{80, 14},
            mFont,
            "#More Info",
            [this]{ ShowMoreInfo(); }
        },
        mHasMoreInfo{},
        mShowingMoreInfo{},
        mMoreInfoDescription{""},
        mDescription{""},
        mFinished{std::move(finished)}
    {
    }

    bool OnMouseEvent(const MouseEvent& event)
    {
        const auto handled = Widget::OnMouseEvent(event);
        if (!handled)
        {
            return std::visit(overloaded{
                [this](const LeftMousePress& p){ return MousePressed(); },
                [this](const RightMousePress& p){ return MousePressed(); },
                [](const auto& p){ return true; }
                },
                event);
        }

        return true;
    } 

    void AddItem(const BAK::InventoryItem& item, BAK::GameState& gameState)
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryIcon(item.GetObject().mImageIndex);
        mItem.SetImage(ss, ti, dims);

        Logging::LogDebug(__FUNCTION__) << " Adding item: " << item << "\n";
        {
            std::stringstream ss{};
            ss << "#" << item.GetObject().mName << "\n";
            if (item.DisplayNumber())
            {
                if (item.DisplayCondition())
                {
                    ss << "Condition: " << item.GetCondition() << "%";
                }
                else
                {
                    ss << "Amount: " << item.GetCondition();
                }
            }
            mName.AddText(mFont, ss.str(), true);
            Logging::LogDebug(__FUNCTION__) << " Name: " << ss.str() << "\n";
        }

        if (item.IsItemType(BAK::ItemType::Scroll))
        {
            mDescription = gameState.GetTextVariableStore()
                .SubstituteVariables(
                    std::string{BAK::DialogSources::GetScrollDescription(item.GetScroll())});
        }
        else
        {
            mDescription = gameState.GetTextVariableStore()
                .SubstituteVariables(
                    std::string{BAK::DialogSources::GetItemDescription(item.GetItemIndex().mValue)});
        }

        mDescriptionText.AddText(mFont, mDescription, true, true);
        mShowingMoreInfo = false;

        {
            std::stringstream ss{};
            bool comma{};
            ss << "#";

            if (item.IsEquipped())
            {
                comma = true;
                ss << "Using";
            }
            if (item.IsBroken())
            {
                if (comma) ss << ", ";
                comma = true;
                ss << "Broken";
            }
            if (item.IsRepairable()
                && (item.IsItemType(BAK::ItemType::Armor)
                    || item.IsItemType(BAK::ItemType::Crossbow)
                    || item.IsItemType(BAK::ItemType::Sword)))
            {
                if (comma) ss << ", ";
                comma = true;
                ss << "Repairable";
            }

            if (HasMoreInfo(item))
            {
                Logging::LogDebug(__FUNCTION__) << " Status: " << ss.str();
                mStatusText.AddText(mFont, ss.str(), true, true);
                mHasMoreInfo = true;

                mMoreInfoDescription = MakeMoreInfo(item);
            }
            else
            {
                mHasMoreInfo = false;
            }
        }

        AddChildren();
    }

private:
    bool HasMoreInfo(const BAK::InventoryItem& item)
    {
        const auto isQuarrel = item.IsItemType(BAK::ItemType::Unspecified)
            && ((item.GetObject().mCategories 
                & static_cast<std::uint16_t>(BAK::SaleCategory::CrossbowRelated)) != 0);

        return item.IsItemType(BAK::ItemType::Sword)
            || item.IsItemType(BAK::ItemType::Armor)
            || item.IsItemType(BAK::ItemType::Crossbow)
            || item.IsItemType(BAK::ItemType::Staff)
            || item.IsItemType(BAK::ItemType::ArmorOil)
            || item.IsItemType(BAK::ItemType::SpecialOil)
            || item.IsItemType(BAK::ItemType::WeaponOil)
            || (item.IsSkillModifier() && item.IsEquipped())
            || isQuarrel;
    }

    void ShowMoreInfo()
    {
        if (mShowingMoreInfo)
        {
            mDescriptionText.AddText(mFont, mDescription, true, true);
            mShowingMoreInfo = false;
        }
        else
        {
            mDescriptionText.AddText(mFont, mMoreInfoDescription, false, true);
            mShowingMoreInfo = true;
        }
    }

    std::string MakeMoreInfo(const BAK::InventoryItem& item)
    {
        std::stringstream ss{};
        const auto& object = item.GetObject();
        const auto GetMods = [&]{
            if (item.HasModifier(BAK::Modifier::Flaming)) return "Flaming";
            if (item.HasModifier(BAK::Modifier::SteelFire)) return "Steelfired";
            if (item.HasModifier(BAK::Modifier::Frost)) return "Frosted";
            if (item.HasModifier(BAK::Modifier::Enhancement1)) return "Enhanced";
            if (item.HasModifier(BAK::Modifier::Enhancement2)) return "Enhanced";
            if (item.IsPoisoned()) return "Poisoned";
            const auto effect = object.mEffectMask >> 8;
            if (CheckBitSet(effect, BAK::Modifier::Flaming)) return "Flame";
            if (CheckBitSet(effect, BAK::Modifier::SteelFire)) return "Steelfire";
            if (CheckBitSet(effect, BAK::Modifier::Frost)) return "Frost";
            if (CheckBitSet(effect, BAK::Modifier::Enhancement1)) return "Enhancement";
            if (CheckBitSet(effect, BAK::Modifier::Enhancement2)) return "Enhancement";
            if (CheckBitSet(object.mEffectMask, BAK::ItemStatus::Poisoned)) return "Poison";
            return "None";
        };

        const auto GetBlessing = [&]{
            if (item.HasModifier(BAK::Modifier::Blessing1)) return "No. 1 (+5%)";
            if (item.HasModifier(BAK::Modifier::Blessing2)) return "No. 2 (+10%)";
            if (item.HasModifier(BAK::Modifier::Blessing3)) return "No. 3 (+15%)";
            return "None";
        };

        if (item.IsItemType(BAK::ItemType::Armor))
        {
            ss << "Armor Mod:   #" << object.mAccuracySwing << "%#\n\n";
            ss << "Resistances: #" << GetMods() << "#\n";
            ss << "Bless Type:  #" << GetBlessing() << "#\n\n";
            ss << "Racial Mod:  #" << BAK::ToString(object.mRace) << "#\n";
        }
        else if (item.IsItemType(BAK::ItemType::Sword))
        {
            ss << "             #" << "Thrust  Swing#\n";
            ss << "Base Dmg:    #" << object.mStrengthThrust << "+Strength " << object.mStrengthSwing << "+Strength#\n";
            ss << "Accuracy:    #" << object.mAccuracyThrust << "+Skill " << object.mAccuracySwing << "+Skill#\n\n";
            ss << "Active Mods: #" << GetMods() << "#\n";
            ss << "Bless Type:  #" << GetBlessing() << "#\n\n";
            ss << "Racial Mod:  #" << BAK::ToString(object.mRace) << "#\n";
        }
        else if (item.IsItemType(BAK::ItemType::Staff))
        {
            ss << "             #" << "Thrust  Swing#\n";
            ss << "Base Dmg:    #" << object.mStrengthThrust << "+Strength " << object.mStrengthSwing << "+Strength#\n";
            ss << "Accuracy:    #" << object.mAccuracyThrust << "+Skill " << object.mAccuracySwing << "+Skill#\n\n";
            ss << "Racial Mod:  #" << BAK::ToString(object.mRace) << "#\n";
        }
        else if (item.IsItemType(BAK::ItemType::Crossbow))
        {
            ss << "Base Damage: #" << object.mStrengthSwing << "+Quarrel#\n";
            ss << "Accuracy:    #" << object.mAccuracySwing<< "+Quarrel+Skill#\n\n";
            ss << "Racial Mod:  #" << BAK::ToString(object.mRace) << "#\n";
        }
        else if (item.IsItemType(BAK::ItemType::Unspecified)
            && ((object.mCategories & static_cast<std::uint16_t>(BAK::SaleCategory::CrossbowRelated)) != 0))
        {
            ss << "Base Damage: #" << object.mStrengthSwing << "#\n";
            ss << "Accuracy:    #" << object.mAccuracySwing << "+Skill#\n\n";
            ss << "Racial Mod:  #" << BAK::ToString(object.mRace) << "#\n";
        }
        else if (item.IsItemType(BAK::ItemType::WeaponOil)
            || item.IsItemType(BAK::ItemType::ArmorOil)
            || item.IsItemType(BAK::ItemType::SpecialOil))
        {
            ss << "Modifier: #" << GetMods() << "#\n";
        }
        else if (item.IsSkillModifier())
        {
            ss << "#Affecting player statistics\n";
        }
        return ss.str();
    }

    bool MousePressed()
    {
        ASSERT(mFinished);
        std::invoke(mFinished);
        return true;
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mName);
        AddChildBack(&mItem);
        AddChildBack(&mDescriptionBackground);
        AddChildBack(&mDescriptionText);
        AddChildBack(&mStatusText);
        if (mHasMoreInfo)
            AddChildBack(&mMoreInfo);
    }

    const Icons& mIcons;
    const Font& mFont;
    TextBox mName;
    CenteredImage mItem;
    TextBox mStatusText;
    Button mDescriptionBackground;
    TextBox mDescriptionText;
    ClickButton mMoreInfo;
    bool mHasMoreInfo;
    bool mShowingMoreInfo;
    std::string mMoreInfoDescription;
    std::string mDescription;
    std::function<void()> mFinished;
};

}
