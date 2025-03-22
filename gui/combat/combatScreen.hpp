#pragma once

#include "audio/audio.hpp"

#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"
#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/dragEndpoint.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/core/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

namespace Gui::Combat {

class CombatScreen : public Widget
{
public:
    static constexpr auto sLayoutFile = "COMBAT.DAT";
    static constexpr auto sShootLayoutFile = "SHOOT.DAT";
    static constexpr auto sBackground = "CFRAME.SCX";

    // Request offsets
    static constexpr auto mContainerTypeRequest = 3;

    static constexpr auto mNextPageButton = 52;
    static constexpr auto mNextPageRequest = 4;

    static constexpr auto mExitRequest = 5;
    static constexpr auto mExitButton = 13;

    static constexpr auto mGoldRequest = 6;

    CombatScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);

    void SetSelectedCharacter(
        BAK::ActiveCharIndex character);

    /* Widget */
    bool OnMouseEvent(const MouseEvent& event) override;

private:
    auto& GetCharacter(BAK::ActiveCharIndex i)
    {
        return mGameState.GetParty().GetCharacter(i);
    }

    void StartDialog(BAK::Target target);
    void RefreshGui();

    void UpdatePartyMembers();
    
    void AddChildren();
    
private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    using CharacterButton = Clickable<
        Clickable<
            Widget,
            RightMousePress,
            std::function<void()>>,
        LeftMousePress,
        std::function<void()>>;

    std::vector<CharacterButton> mCharacters;
    ClickButtonImage mExit;

    std::optional<BAK::ActiveCharIndex> mSelectedCharacter;
    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
