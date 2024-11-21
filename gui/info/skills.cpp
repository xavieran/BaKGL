#include "gui/info/skills.hpp"

#include "bak/layout.hpp"
#include "bak/skills.hpp"

#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include "com/visit.hpp"

#include <glm/glm.hpp>

#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace Gui {

Blood::Blood(
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

void Blood::UpdateValue(unsigned value)
{
    const auto& pos = mImage.GetPositionInfo().mPosition;
    const auto& dims = GetPositionInfo().mDimensions;
    SetPosition(glm::vec2{pos.x + 101 - value, pos.y});
    SetDimensions(glm::vec2{value, dims.y});
}

Skill::Skill(
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

void Skill::UpdateValue(
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
    mText.SetText(font, ss.str());
    mBlood.UpdateValue(value);

    if (skillSelected)
        mSkillSelected = true;
    else
        mSkillSelected = false;
        
    AddChildren();
}

void Skill::AddChildren()
{
    ClearChildren();

    AddChildBack(&mText);
    AddChildBack(&mBlood);
    if (mSkillSelected)
        AddChildBack(&mSelected);
}

bool Skill::OnMouseEvent(const MouseEvent& event)
{
    return std::visit(overloaded{
        [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
        [](const auto& p){ return false; }
        },
        event);
}

bool Skill::LeftMousePressed(glm::vec2 click)
{
    if (Within(click))
    {
        std::invoke(mToggleSkillSelected);
        return true;
    }

    return false;
}

Skills::Skills(
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
    mToggleSkillSelected{std::move(toggleSkill)}
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

bool Skills::OnMouseEvent(const MouseEvent& event)
{
    const auto result = Widget::OnMouseEvent(event);
    ClickButtonBase::OnMouseEvent(event);
    return result;
}

void Skills::UpdateSkills(
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

}
