#pragma once

#include "gui/core/widget.hpp"

namespace Gui {
class Icons;
}

namespace Gui::Camp::detail {

class CampDest : public Widget
{
    static constexpr auto sBlank = 0;
    static constexpr auto sUnlit = 1;
    static constexpr auto sHighlighted = 2;
    static constexpr auto sCurrent = 3;
    static constexpr auto sBlank2 = 4;

public:
    CampDest(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(bool)>&& selected);

    bool GetCurrent() const;
    void SetCurrent(bool current);
    void SetHighlighted();

public:
    void Entered();
    void Exited();

    const Icons& mIcons;
    bool mCurrent;
    std::function<void(bool)> mCallback;
};

}
