#pragma once

#include "bak/layout.hpp"
#include "bak/skills.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/colors.hpp"
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"
#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>
#include <vector>

namespace Gui {

class Blood : public Widget
{
public:
    Blood(
        glm::vec2 pos,
        glm::vec2 dims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex image)
    :
        Widget{
            ClipRegionTag{},
            pos,
            dims,
            false 
        },
        mImage{
            ImageTag{},
            spriteSheet,
            image,
            glm::vec2{pos},
            glm::vec2{101, 4},
            false 
        }
    {
        AddChildBack(&mImage);
    }

    void UpdateValue(unsigned value)
    {
        const auto& pos = mImage.GetPositionInfo().mPosition;
        const auto& dims = GetPositionInfo().mDimensions;
        SetPosition(glm::vec2{pos.x + 101 - value, pos.y});
        SetDimensions(glm::vec2{value, dims.y});
    }

    Widget mImage;
};

class Skill : public Widget
{
public:
    Skill(
        glm::vec2 pos,
        glm::vec2 dims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex swordImage,
        Graphics::TextureIndex bloodImage,
        Graphics::TextureIndex selectedImage,
        std::function<void()>&& toggleSkillSelected)
    :
        Widget{
            ImageTag{},
            spriteSheet,
            swordImage,
            pos,
            dims,
            true
        },
        mText{
            glm::vec2{34, -5},
            glm::vec2{100, 16}
        },
        mBlood{
            glm::vec2{31, 5},
            glm::vec2{101, 4},
            spriteSheet,
            bloodImage},
        mSelected{
            ImageTag{},
            spriteSheet,
            selectedImage,
            glm::vec2{-1, 5},
            glm::vec2{5,5},
            false
        },
        mSkillSelected{false},
        mToggleSkillSelected{std::move(toggleSkillSelected)}
    {
        AddChildren();
    }

    void UpdateValue(
        const Font& font,
        BAK::SkillType skill,
        unsigned value,
        bool skillSelected,
        bool unseenIprovement)
    {
        const auto skillStr = BAK::ToString(skill);
        std::stringstream ss;
        if (unseenIprovement)
            ss << "\xf5";
        else
            ss << "#";

        ss << skillStr << std::setw(18 - skillStr.size()) 
            << std::setfill(' ') << value << "%";
        mText.AddText(font, ss.str());
        mBlood.UpdateValue(value);

        if (skillSelected)
            mSkillSelected = true;
        else
            mSkillSelected = false;
            
        AddChildren();
    }

    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mText);
        AddChildBack(&mBlood);
        if (mSkillSelected)
            AddChildBack(&mSelected);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            std::invoke(mToggleSkillSelected);
            return true;
        }

        return false;
    }

    TextBox mText;
    Blood mBlood;
    Widget mSelected;
    bool mSkillSelected;
    std::function<void()> mToggleSkillSelected;
};

class Skills : public ClickButtonBase
{
public:
    static constexpr auto sSkillWidgetStart = 3;
    static constexpr auto sSelectableSkillsOffset = 4;

    Skills(
        glm::vec2 pos,
        glm::vec2 dims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex spriteOffset,
        const BAK::Layout& layout,
        std::function<void(BAK::SkillType)>&& toggleSkill,
        std::function<void()>&& onRightMousePress)
    :
        ClickButtonBase{
            pos,
            dims,
            []{},
            std::move(onRightMousePress)
        },
        mSkills{},
        mToggleSkillSelected{std::move(toggleSkill)},
        mLogger{Logging::LogState::GetLogger("Gui::Skills")}
    {
        mSkills.reserve(layout.GetSize());
        for (unsigned i = sSkillWidgetStart; i < layout.GetSize(); i++)
        {
            auto& s = mSkills.emplace_back(
                layout.GetWidgetLocation(i) - pos + glm::vec2{0, 8},
                layout.GetWidgetDimensions(i),
                spriteSheet,
                Graphics::TextureIndex{spriteOffset.mValue + 21},
                Graphics::TextureIndex{spriteOffset.mValue + 22},
                Graphics::TextureIndex{spriteOffset.mValue + 23},
                [this, skill=static_cast<BAK::SkillType>(i +1)](){
                    mToggleSkillSelected(skill);
                });
        }

        for (auto& skill : mSkills)
            AddChildBack(&skill);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        const auto result = Widget::OnMouseEvent(event);
        ClickButtonBase::OnMouseEvent(event);
        return result;
    }

    void UpdateSkills(
        const Font& font,
        const BAK::Skills& skills)
    {
        for (unsigned i = sSelectableSkillsOffset; i < BAK::Skills::sSkills; i++)
        {
            const auto skill = skills.GetSkill(static_cast<BAK::SkillType>(i));
            mSkills[i - sSelectableSkillsOffset].UpdateValue(
                font,
                static_cast<BAK::SkillType>(i),
                skill.mCurrent,
                skill.mSelected,
                skill.mUnseenImprovement);
        }
    }

private:
    std::vector<Skill> mSkills;

    std::function<void(BAK::SkillType)> mToggleSkillSelected;

    const Logging::Logger& mLogger;
};

}
