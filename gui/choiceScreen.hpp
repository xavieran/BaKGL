#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/algorithm.hpp"
#include "com/visit.hpp"

#include "gui/clickButton.hpp"
#include "gui/colors.hpp"
#include "gui/frame.hpp"
#include "gui/screenStack.hpp"
#include "gui/widget.hpp"

namespace Gui {

class ChoiceScreen : public Widget
{
public:
    using ChoiceIndex = unsigned;
    using FinishedCallback = std::function<void(ChoiceIndex)>;

    static constexpr auto mMaxChoices = 16;
    ChoiceScreen(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        ScreenStack& screenStack,
        FinishedCallback&& finished)
    :
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            pos,
            dims,
            true
        },
        mButtons{},
        mFont{font},
        mScreenStack{screenStack},
        mFinished{std::move(finished)},
        mLogger{Logging::LogState::GetLogger("Gui::ChoiceScreen")}
    {
        assert(mFinished);
        // there's never more than this many choices
        mButtons.reserve(mMaxChoices);
    }

    void StartChoices(
        std::vector<std::pair<ChoiceIndex, std::string>> choices,
        glm::vec2 buttonSize)
    {
        ClearChildren();
        mButtons.clear();

        assert(choices.size() <= mMaxChoices);

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

    void Choose(ChoiceIndex index)
    {
        std::invoke(mFinished, index);
        mButtons.clear();
    }

private:
    std::vector<ClickButton> mButtons;
    const Font& mFont;
    ScreenStack& mScreenStack;

    FinishedCallback mFinished;

    const Logging::Logger& mLogger;
};

}
