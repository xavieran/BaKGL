#include "gui/choiceScreen.hpp"

#include "bak/types.hpp"

#include "com/assert.hpp"

#include "gui/clickButton.hpp"
#include "gui/fontManager.hpp"
#include "gui/screenStack.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

ChoiceScreen::ChoiceScreen(
    glm::vec2 pos,
    glm::vec2 dims,
    const Font& font,
    ScreenStack& screenStack,
    FinishedCallback&& finished)
:
    Widget{
        RectTag{},
        pos,
        dims,
        glm::vec4{0},
        true
    },
    mButtons{},
    mFont{font},
    mScreenStack{screenStack},
    mFinished{std::move(finished)}
{
    ASSERT(mFinished);
    // there's never more than this many choices
    mButtons.reserve(mMaxChoices);
}

void ChoiceScreen::StartChoices(
    const std::vector<std::pair<BAK::ChoiceIndex, std::string>>& choices,
    glm::vec2 buttonSize)
{
    ClearChildren();
    mButtons.clear();

    ASSERT(choices.size() <= mMaxChoices);

    auto pos = glm::vec2{0};
    const auto margin = 5;
    const auto limit = GetPositionInfo().mDimensions;
    for (const auto& [index, label] : choices)
    {
        mButtons.emplace_back(
            pos,
            buttonSize,
            mFont,
            label,
            [this, choice=index](){
                Choose(choice);
            });
        pos.x += buttonSize.x + margin;
        // if the next button would overflow...
        if ((pos.x + buttonSize.x) > limit.x)
        {
            pos.x = 0;
            pos.y += buttonSize.y + margin;
        }
    }

    for (auto& button : mButtons)
        AddChildBack(&button);
}

void ChoiceScreen::Choose(BAK::ChoiceIndex index)
{
    std::invoke(mFinished, index);
}
}
