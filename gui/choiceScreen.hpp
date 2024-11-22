#pragma once

#include "bak/types.hpp"

#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class Font;
class ScreenStack;

class ChoiceScreen : public Widget
{
public:
    using FinishedCallback = std::function<void(BAK::ChoiceIndex)>;

    static constexpr auto mMaxChoices = 16;
    ChoiceScreen(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        ScreenStack& screenStack,
        FinishedCallback&& finished);
    
    void StartChoices(
        const std::vector<std::pair<BAK::ChoiceIndex, std::string>>& choices,
        glm::vec2 buttonSize);
    void Choose(BAK::ChoiceIndex index);
private:
    std::vector<ClickButton> mButtons;
    const Font& mFont;
    ScreenStack& mScreenStack;

    FinishedCallback mFinished;
};

}
