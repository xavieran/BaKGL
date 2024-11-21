#pragma once

#include "bak/layout.hpp"
#include "bak/skills.hpp"

#include "graphics/types.hpp"

#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace Gui {

class Blood : public Widget
{
public:
    Blood(
        glm::vec2 pos,
        glm::vec2 dims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex image);
    
    void UpdateValue(unsigned value);

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
        std::function<void()>&& toggleSkillSelected);
    
    void UpdateValue(
        const Font& font,
        BAK::SkillType skill,
        unsigned value,
        bool skillSelected,
        bool unseenIprovement);

    bool OnMouseEvent(const MouseEvent& event) override;
    bool LeftMousePressed(glm::vec2 click);

    void AddChildren();

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
        std::function<void()>&& onRightMousePress);

    bool OnMouseEvent(const MouseEvent& event) override;

    void UpdateSkills(
        const Font& font,
        const BAK::Skills& skills);

private:
    std::vector<Skill> mSkills;

    std::function<void(BAK::SkillType)> mToggleSkillSelected;
};

}
