#include "gui/townLabel.hpp"

namespace Gui::detail {

TownLabelBase::TownLabelBase(
    glm::vec2 pos,
    glm::vec2 dims,
    const Font& font,
    const std::string& label)
:
    Widget{
        RectTag{},
        pos,
        dims,
        glm::vec4{0},
        true
    },
    mTown{label},
    mLabel{{}, glm::vec2{120, 32}}
{
    const auto& [tDims, _] = mLabel.SetText(font, label);
    mLabel.SetDimensions(tDims);
    mLabel.SetCenter(glm::vec2{0, -3});
}

void TownLabelBase::Entered()
{
    ClearChildren();
    AddChildBack(&mLabel);
}

void TownLabelBase::Exited()
{
    ClearChildren();
}

}
