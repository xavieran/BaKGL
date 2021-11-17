#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/types.hpp"

#include "com/algorithm.hpp"
#include "com/assert.hpp"
#include "com/ostream.hpp"
#include "com/visit.hpp"

#include "gui/clickButton.hpp"
#include "gui/colors.hpp"
#include "gui/screenStack.hpp"
#include "gui/widget.hpp"

namespace Gui {

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
        mFinished{std::move(finished)},
        mLogger{Logging::LogState::GetLogger("Gui::ChoiceScreen")}
    {
        ASSERT(mFinished);
        // there's never more than this many choices
        mButtons.reserve(mMaxChoices);
    }

    void StartChoices(
        const std::vector<std::pair<BAK::ChoiceIndex, std::string>>& choices,
        glm::vec2 buttonSize)
    {
        mLogger.Debug() << __FUNCTION__ << " choices: " << choices << "\n";
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

    void Choose(BAK::ChoiceIndex index)
    {
        std::invoke(mFinished, index);
    }

private:
    std::vector<ClickButton> mButtons;
    const Font& mFont;
    ScreenStack& mScreenStack;

    FinishedCallback mFinished;

    const Logging::Logger& mLogger;
};

}
