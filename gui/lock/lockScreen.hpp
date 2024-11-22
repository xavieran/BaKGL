#pragma once

#include "audio/audio.hpp"

#include "bak/dialogTarget.hpp"
#include "bak/layout.hpp"

#include "gui/lock/lock.hpp"

#include "gui/inventory/containerDisplay.hpp"
#include "gui/inventory/details.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/core/clickable.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class IContainer;

class Character;
class GameState;
}

namespace Gui {

class IGuiManager;
class Backgrounds;
class Font;
class InventorySlot;

class LockScreen :
    public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_INV.DAT";
    static constexpr auto sBackground = "INVENTOR.SCX";

    static constexpr auto sPickBrokeSound = AudioA::SoundIndex{0x5};
    static constexpr auto sPickedLockSound = AudioA::SoundIndex{0x16};
    static constexpr auto sUseKeySound = AudioA::SoundIndex{0x1e};
    static constexpr auto sKeyBrokeSound  = AudioA::SoundIndex{0x2b};
    static constexpr auto sOpenLockSound  = AudioA::SoundIndex{30};

    // Request offsets
    static constexpr auto mContainerTypeRequest = 3;

    static constexpr auto mNextPageButton = 52;
    static constexpr auto mNextPageRequest = 4;

    static constexpr auto mExitRequest = 5;
    static constexpr auto mExitButton = 13;

    static constexpr auto mGoldRequest = 6;

    LockScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);

    void SetSelectedCharacter(
        BAK::ActiveCharIndex character);

    void ResetUnlocked();
    bool IsUnlocked() const;
    BAK::IContainer* GetContainer() const;
    void SetContainer(BAK::IContainer* container);

    /* Widget */
    bool OnMouseEvent(const MouseEvent& event) override;
    void PropagateUp(const DragEvent& event) override;
private:
    void RefreshGui();
    BAK::Character& GetCharacter(BAK::ActiveCharIndex i);
    void ShowLockDescription();
    void AttemptLock(const InventorySlot& itemSlot);
    void Unlocked(BAK::KeyTarget dialog);
    void ShowItemDescription(const BAK::InventoryItem& item);
    void ExitDetails();
    void UpdatePartyMembers();
    void UpdateGold();
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
    TextBox mGoldDisplay;
    
    Details mDetails;
    bool mDisplayDetails;
    Clickable<
        ItemEndpoint<Lock>,
        RightMousePress,
        std::function<void()>> mLock;
    ClickButtonImage mContainerTypeDisplay;
    ContainerDisplay mContainerScreen;

    std::optional<BAK::ActiveCharIndex> mSelectedCharacter;
    BAK::IContainer* mContainer;
    bool mNeedRefresh;

    bool mUnlocked;

    const Logging::Logger& mLogger;
};

}
