#pragma once

#include "bak/constants.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"
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
    Keywords();

    std::string_view GetDialogChoice(unsigned i) const;
    std::string_view GetQueryChoice(unsigned i) const;
    std::string_view GetNPCName(unsigned i) const;

    static constexpr auto sYesIndex = 0x100;
    static constexpr auto sNoIndex  = 0x101;
private:
    static constexpr auto mCharacterNameOffset = 45;
    static constexpr auto mDialogChoiceOffset = 98;
    static constexpr auto mQueryChoiceOffset = 1;

    std::vector<std::string> mPartyMembers;
    std::vector<std::string> mKeywords;
};

enum class DisplayFlags
{
    Fullscreen = 0x6,
};

enum class DialogStyle
{
    Fullscreen,
    ActionArea, 
    LowerArea,
    Popup
};

enum class TextStyle
{
    VerticalCenter = 1,
    HorizontalCenter = 2,
    IsBold = 4
};

enum class ChoiceStyle
{
    EvaluateState,
    Prompt, // e.g. display yes/no buttons
    Dialog, // display "Locklear asked about: "
    Random  // pick a random choice
};

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
    // 0x05 -> In large action/game part of screen (e.g. temple iface)
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
    // 0x1 -> Show in action part of screen...
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

}
