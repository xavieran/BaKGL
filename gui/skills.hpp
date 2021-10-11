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
        Graphics::TextureIndex selectedImage)
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
            glm::vec2{34, -6},
            glm::vec2{100, 16}
        },
        mBlood{
            glm::vec2{31, 5},
            glm::vec2{101, 4},
            spriteSheet,
            bloodImage},
        mSelected{}
    {
        AddChildBack(&mText);
        AddChildBack(&mBlood);
        //mSelected.emplace(
        //    ImageTag{},
        //    spriteSheet,
        //    selectedImage,
        //    glm::vec2{-2, 4},
        //    glm::vec2{6,7},
        //    false);
        //AddChildBack(&(*mSelected));
    }

    void UpdateValue(
        const Font& font,
        BAK::SkillType skill,
        unsigned value)
    {
        const auto skillStr = BAK::ToString(skill);
        std::stringstream ss;
        ss << skillStr << std::setw(18 - skillStr.size()) 
            << std::setfill(' ') << value << "%";
        mText.AddText(font, ss.str());
        mBlood.UpdateValue(value);
    }

    TextBox mText;
    Blood mBlood;
    std::optional<Widget> mSelected;
};

class Skills : public Widget
{
public:

    Skills(
        Graphics::SpriteSheetIndex spriteSheet,
        unsigned spriteOffset,
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

            auto& s = mSkills.emplace_back(
                glm::vec2{x, y},
                glm::vec2{data.width, data.height},
                spriteSheet,
                spriteOffset + 21,
                spriteOffset + 22,
                spriteOffset + 23);
                    }

        for (auto& skill : mSkills)
            AddChildBack(&skill);
    }

    void UpdateSkills(
        const Font& font,
        const BAK::Skills& skills)
    {
        for (unsigned i = 4; i < BAK::Skills::sSkills; i++)
        {
            mSkills[i - 4].UpdateValue(
                font,
                static_cast<BAK::SkillType>(i),
                skills.mSkills[i].mCurrent);
        }
    }

private:
    std::vector<Skill> mSkills;

    const Logging::Logger& mLogger;
};

}
