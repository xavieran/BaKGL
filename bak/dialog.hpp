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
    void Load(FileBuffer& fb)
    {
        auto length = fb.GetUint16LE();
        std::cout << "Loading keywords" << std::endl;
        std::cout << "Length: " << length << std::endl;

        unsigned i = 0;
        while (fb.Tell() != 0x2b8)
        {
            std::cout << "I: " << i << " " << std::hex << fb.GetUint16LE()
                << std::dec << std::endl;
            i++;
        }
        i = 0;
        while (fb.GetBytesLeft() != 0)
        {
            std::cout << "Str:" << i << " :: " << fb.GetString() << std::endl;
            i++;
        }
    }
};

enum class ChoiceState
{
    Money   = 0x7531,
    Chapter = 0x7537,

};

class DialogSnippet
{
public:
    struct Choice
    {
        Choice(
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

    struct Action
    {
        Action(
            const std::array<std::uint8_t, 10>& action)
        :
            mAction{action}
        {}

        std::array<std::uint8_t, 10> mAction;
    };

    DialogSnippet(FileBuffer& fb, std::uint8_t dialogFile)
    {
        mDisplayStyle        = fb.GetUint8();
        mActor               = fb.GetUint16LE();
        mDisplayStyle2       = fb.GetUint8();
        mDisplayStyle3       = fb.GetUint8();
        std::uint8_t choices = fb.GetUint8();
        std::uint8_t actions = fb.GetUint8();
        auto length          = fb.GetUint16LE();
        
        std::uint8_t i;

        const auto GetTarget = [dialogFile](auto rawTarget) -> Target
        {
            constexpr std::uint32_t targetBit = 0x80000000;
            if (targetBit & rawTarget)
                return KeyTarget{rawTarget & (targetBit - 1)};
            else
                return OffsetTarget{dialogFile, rawTarget};
        };

        for (i = 0; i < choices; i++)
        {
            const auto state   = fb.GetUint16LE();
            const auto choice1 = fb.GetUint16LE();
            const auto choice2 = fb.GetUint16LE();
            const auto offset  = fb.GetUint32LE();
            const auto target  = GetTarget(offset);
            // FIXME: Should work out what to do with offset == 0
            if (offset != 0)
                mChoices.emplace_back(state, choice1, choice2, target);
        }

        for (i = 0; i < actions; i++)
        {
            mActions.emplace_back(fb.GetArray<10>());
        }
        
        if (length > 0)
            mText = fb.GetString(length);
        else
            mText = "";
    }


    const std::vector<Choice>& GetChoices() const { return mChoices; }
    std::string_view GetText() const { return mText; }

    std::uint8_t mDisplayStyle;
    std::uint16_t mActor;
    std::uint8_t mDisplayStyle2;
    std::uint8_t mDisplayStyle3;

    std::vector<Choice> mChoices;
    std::vector<Action> mActions;
    std::string mText;
};

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d);

class DialogStore
{
public:
    DialogStore();

    void Load();

    void ShowAllDialogs();
    void ShowDialog(Target dialogKey);

    const DialogSnippet& GetSnippet(Target target) const;

    bool HasSnippet(Target target) const;

    OffsetTarget GetTarget(KeyTarget dialogKey);

    std::string_view GetFirstText(const DialogSnippet& snippet) const;

    const DialogSnippet& operator()(KeyTarget dialogKey) const;
    const DialogSnippet& operator()(OffsetTarget snippetKey) const;

private:
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

    DialogIndex(const ZoneLabel& zoneLabel);

    void Load();

    const auto& GetKeys() const { return mKeys; }

private:
    std::vector<Target> mKeys;

    const ZoneLabel& mZoneLabel;
    const Logging::Logger& mLogger;
};

}
