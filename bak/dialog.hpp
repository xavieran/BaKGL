#pragma once

#include "bak/constants.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/resourceNames.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

#include <cassert>
#include <iomanip>
#include <ostream>
#include <unordered_map>

namespace BAK {

class Keywords
{
public:
    void Load(FileBuffer& fb);
};

enum class ChoiceState
{
    Money   = 0x7531,
    Chapter = 0x7537,
    Time    = 0x7539,
    // e.g. KeyTarget{1b7767} Repair, Flterchers Post == 4
    ShopType = 0x7542,
};

enum class DisplayFlags
{
    Fullscreen = 0x6,
};

enum class DialogResult
{
    // Maybe also something to do wih tstate?
    Unknown1  = 0x01,
    GiveItem  = 0x02,
    LoseItem  = 0x03, // pt2 object, pt3 amount
    // Unlocks dialog options / Sets event state
    SetFlag   = 0x04,
    GainSkill = 0x09,
    // Maybe?
    PlaySound = 0xc,
    // something to with state?
    Unknown   = 0x10,
};

struct DialogChoice
{
    DialogChoice(
        std::uint16_t state,
        std::uint16_t choice1,
        std::uint16_t choice2,
        Target target)
    :
        mState{state},
        mChoice1{choice1},
        mChoice2{choice2},
        mTarget{target}
    {}

    std::uint16_t mState;
    std::uint16_t mChoice1;
    std::uint16_t mChoice2;
    Target mTarget;
};

std::ostream& operator<<(std::ostream&, const DialogChoice&);

struct DialogAction
{
    DialogAction(
        const std::array<std::uint8_t, 8>& rest,
        std::uint16_t type)
    :
        mType{static_cast<DialogResult>(type)},
        mRest{rest}
    {}

    DialogResult mType;
    std::array<std::uint8_t, 8> mRest;
};

std::ostream& operator<<(std::ostream& os, const DialogAction& d);

class DialogSnippet
{
public:
    DialogSnippet(FileBuffer& fb, std::uint8_t dialogFile);

    const auto& GetChoices() const { return mChoices; }
    std::string_view GetText() const { return mText; }

    // Display Style One - where to display text
    // 0x00 -> Center of full screen
    // 0x02 -> In action/game part of screen
    // 0x03 -> In non-bold at bottom
    // 0x04 -> In bold at bottom
    // 0x06 -> Center of full screen
    std::uint8_t mDisplayStyle;
    // Actor <= 0x6 => Show alternate background (player character is talking...)
    // Actor > 0x6 show normal background
    // Actor == 0xff party leader is talking
    std::uint16_t mActor;
    // 0x10 Vertically centered, background is with flowers, text always scrolls in
    // 0x04 Vertically and horizontally centered
    // 0x03 Show background
    std::uint8_t mDisplayStyle2;

    // Display Style 3:
    // 0x0 -> No choices
    // 0x2 -> Lay choices side by side (e.g. Yes/No prompt)
    // 0x4 -> (Dialog tree root... Character "asked about") 
    //       -> Choices in grid
    //       -> Goodbye is special and immediately quits the dialog
    std::uint8_t mDisplayStyle3;

    std::vector<DialogChoice> mChoices;
    std::vector<DialogAction> mActions;
    std::string mText;
};

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d);

class DialogStore
{
public:
    DialogStore();

    void ShowAllDialogs();
    void ShowDialog(Target dialogKey);

    const DialogSnippet& GetSnippet(Target target) const;

    bool HasSnippet(Target target) const;

    OffsetTarget GetTarget(KeyTarget dialogKey);

    std::string_view GetFirstText(const DialogSnippet& snippet) const;

    const DialogSnippet& operator()(KeyTarget dialogKey) const;
    const DialogSnippet& operator()(OffsetTarget snippetKey) const;

private:
    void Load();

    std::string GetDialogFile(std::uint8_t i);

    std::unordered_map<
        KeyTarget,
        OffsetTarget> mDialogMap;

    std::unordered_map<
        OffsetTarget,
        DialogSnippet> mSnippetMap;

    const Logging::Logger& mLogger;
};

class DialogIndex
{
public:
    DialogIndex();

    const auto& GetKeys() const { return mKeys; }

private:
    void Load();

    std::vector<Target> mKeys;

    const Logging::Logger& mLogger;
};

}
