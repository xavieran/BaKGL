#pragma once

#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

#include "gui/textBox.hpp"

namespace Gui {

class Font;

namespace detail {

class TownLabelBase : public Widget
{
public:
    TownLabelBase(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const std::string& label);

    void Entered();
    void Exited();

private:
    std::string mTown;
    TextBox mLabel;
};

}

using TownLabel = Highlightable<detail::TownLabelBase, false>;

}
