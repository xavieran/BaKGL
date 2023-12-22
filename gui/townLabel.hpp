#pragma once

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

#include "gui/colors.hpp"
#include "gui/cursor.hpp"
#include "gui/textBox.hpp"

namespace Gui {

namespace detail {

class TownLabelBase : public Widget
{
public:
    TownLabelBase(
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

    void Entered()
    {
        ClearChildren();
        AddChildBack(&mLabel);
    }

    void Exited()
    {
        ClearChildren();
    }

private:
    std::string mTown;
    TextBox mLabel;
};

}

using TownLabel = Highlightable<detail::TownLabelBase, false>;

}
