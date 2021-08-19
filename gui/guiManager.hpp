#pragma once

#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"

#include "gui/colors.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/hotspot.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GuiManager : public Widget
{
public:

    GuiManager(
        Graphics::SpriteManager& spriteManager,
        DialogRunner& dialogRunner)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            std::invoke([&spriteManager]{
                const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
                return sheetIndex;
            }),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
            false
        },
        mDialogRunner{dialogRunner},
        mLogger{Logging::LogState::GetLogger("Gui::GuiManager")}
    {
    }

    void RunGoto(const BAK::HotspotRef& hotspot)
    {
    }

    void RunDialog(BAK::KeyTarget dialogTarget)
    {
    }

    void RunContainer(BAK::KeyTarget dialogTarget)
    {
    }

    DialogRunner& mDialogRunner;

    const Logging::Logger& mLogger;
};

}
