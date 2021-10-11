#pragma once

#include "bak/skills.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/colors.hpp"
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"

#include "xbak/RequestResource.h"

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
            true
        },
        mImage{
            ImageTag{},
            spriteSheet,
            image,
            glm::vec2{0},
            glm::vec2{101, 4},
            true
        }
    {
        AddChildBack(&mImage);
    }

    Widget mImage;
};

class Skill : public Widget
{
public:
    Skill(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& mFont,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex swordImage,
        Graphics::TextureIndex bloodImage,
        Graphics::TextureIndex selectedImage,
        BAK::SkillType skill,
        unsigned skillValue)
    :
        Widget{
            ImageTag{},
            spriteSheet,
            swordImage,
            pos,
            dims,
            true
        },
        mBlood{
            glm::vec2{31, 5},
            glm::vec2{101, 4},
            spriteSheet,
            bloodImage}
    {
        AddChildBack(&mBlood);
    }

    Blood mBlood;
};

class Skills : public Widget
{
public:

    Skills(
        Graphics::SpriteSheetIndex spriteSheet,
        unsigned spriteOffset,
        const Font& font,
        const RequestResource& request)
    :
        Widget{
            Graphics::DrawMode::Rect,
            Graphics::SpriteSheetIndex{0},
            Graphics::TextureIndex{0},
            Graphics::ColorMode::SolidColor,
            Color::debug,
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mSkills{},
        mLogger{Logging::LogState::GetLogger("Gui::Skills")}
    {
        mSkills.reserve(request.GetSize());
        for (unsigned i = 3; i < request.GetSize(); i++)
        {
            const auto& data = request.GetRequestData(i);
            int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
            int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();

            mSkills.emplace_back(
                glm::vec2{x, y},
                glm::vec2{data.width, data.height},
                font,
                spriteSheet,
                spriteOffset + 21,
                spriteOffset + 22,
                spriteOffset + 23,
                BAK::SkillType::Armorcraft,
                100);
        }

        for (auto& skill : mSkills)
            AddChildBack(&skill);
    }

private:
    std::vector<Skill> mSkills;

    const Logging::Logger& mLogger;
};

}
