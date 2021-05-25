#pragma once

#include "constants.hpp"
#include "dialogTarget.hpp"
#include "resourceNames.hpp"
#include "logger.hpp"
#include "ostream.hpp"

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
    const std::string& GetText() const { return mText; }

    std::uint8_t mDisplayStyle;
    std::uint16_t mActor;
    std::uint8_t mDisplayStyle2;
    std::uint8_t mDisplayStyle3;

    std::vector<Choice> mChoices;
    std::vector<Action> mActions;
    std::string mText;
};

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d)
{
    os << "[ ds: " << std::hex << +d.mDisplayStyle << " act: " << +d.mActor
        << " ds2: " << +d.mDisplayStyle2 << " ds3: " << +d.mDisplayStyle3
        << " ]" << std::endl;
    
    for (const auto& action : d.mActions)
    {
        os << "++ " << action.mAction << std::endl;
    }

    for (const auto& choice : d.mChoices)
    {
        os << ">> " << choice.mState << " -> " << choice.mChoice1 
            << " | " << choice.mChoice2 << " " << choice.mTarget
            << std::endl;
    }

    os << "Text [ " << d.mText << " ]" << std::endl;
    os << "Next [ ";
    if (d.mChoices.size() > 0)
        os << d.mChoices.back().mTarget;
    else
        os << "None";
    os << " ]" << std::endl;
    os << std::dec;

    return os;
};

class DialogStore
{
public:
    DialogStore()
    :
        mDialogMap{},
        mSnippetMap{},
        mLogger{Logging::LogState::GetLogger("DialogStore")}
    {}

    void Load()
    {
        for (std::uint8_t dialogFile = 0; dialogFile < 32; dialogFile++)
        {
            auto fname = GetDialogFile(dialogFile);
            auto fb = FileBufferFactory::CreateFileBuffer(fname);
            unsigned dialogs = fb.GetUint16LE();
            mLogger.Info() << "Dialog " << fname << " has: " << dialogs << " dialogs" << std::endl;

            for (unsigned i = 0; i < dialogs; i++)
            {
                auto key = KeyTarget{fb.GetUint32LE()};
                const auto val = OffsetTarget{dialogFile, fb.GetUint32LE()};
                const auto [it, emplaced] = mDialogMap.emplace(
                    key,
                    val);
                assert(emplaced);
                auto [checkF, checkV] = it->second;
                mLogger.Spam() << std::hex << "0x" << it->first 
                    << " -> 0x" << checkV << std::dec << std::endl;
            }

            while (fb.GetBytesLeft() > 0)
            {
                const auto offset = OffsetTarget{dialogFile, fb.Tell()};
                auto snippet = DialogSnippet{fb, dialogFile};
                mLogger.Spam() << offset << " @ " << snippet << std::endl;
                const auto& [it, emplaced] = mSnippetMap.emplace(offset, snippet);
            }
        }
    }

    void ShowAllDialogs()
    {
        for (const auto& dialogKey : mDialogMap)
        {
            try
            {
                ShowDialog(dialogKey.first);
            }
            catch (const std::runtime_error&)
            {
                mLogger.Error() << "Failed to walk dialog tree: " << dialogKey.first << std::endl;
            }
        }
    }

    void ShowDialog(Target dialogKey)
    {
        auto snippet = std::visit(*this, dialogKey);
        bool noText = true;
        mLogger.Info() << "Dialog for key: " << std::hex << dialogKey << std::endl;
        mLogger.Debug() << "Text: " << GetFirstText(snippet) << std::endl;
    }

    const DialogSnippet& GetSnippet(Target target) const
    {
        return std::visit(*this, target);
    }

    bool HasSnippet(Target target) const
    {
        try
        {
            std::visit(*this, target);
            return true;
        }
        catch (std::runtime_error& e)
        {
            return false;
        }
    }

    OffsetTarget GetTarget(KeyTarget dialogKey)
    {
        auto it = mDialogMap.find(dialogKey);
        if (it == mDialogMap.end())
        {
            std::stringstream err{};
            err << "Key not found: " << std::hex << dialogKey << std::dec;
            throw std::runtime_error(err.str());
        }
        return it->second;
    }

    std::string GetFirstText(const DialogSnippet& snippet) const
    {
        if (snippet.GetText().length() != 0)
            return snippet.GetText();
        else
        {
            if (snippet.GetChoices().size() > 0)
                return GetFirstText(
                    GetSnippet(snippet.GetChoices().front().mTarget));
            else
                return "* no text *";
        }
    }

    const DialogSnippet& operator()(KeyTarget dialogKey) const 
    {
        auto it = mDialogMap.find(dialogKey);
        if (it == mDialogMap.end())
        {
            std::stringstream err{};
            err << "Key not found: " << std::hex << dialogKey << std::dec;
            throw std::runtime_error(err.str());
        }
        return (*this)(it->second);
    }

    const DialogSnippet& operator()(OffsetTarget snippetKey) const
    {
        auto snip = mSnippetMap.find(snippetKey);
        if (snip == mSnippetMap.end())
        {
            std::stringstream err{};
            err << "Offset not found: " << std::hex << snippetKey << std::dec;
            throw std::runtime_error(err.str());
        }
        return snip->second;
    }

    std::string GetDialogFile(std::uint8_t i)
    {
        std::stringstream ss{};
        ss << "DIAL_Z" << std::setw(2) << std::setfill('0') << +i << ".DDX";
        return ss.str();
    }

private:
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

    DialogIndex(const ZoneLabel& zoneLabel)
    :
        mKeys{},
        mZoneLabel{zoneLabel},
        mLogger{Logging::LogState::GetLogger("DialogIndex")}
    {
    }

    void Load()
    {
        BAK::DialogStore dialogStore{};
        dialogStore.Load();

        auto fb = FileBufferFactory::CreateFileBuffer(mZoneLabel.GetDialogPointers());

        const unsigned dialogs = fb.GetUint16LE();
        mLogger.Debug() << "Loading: " << dialogs << std::endl;

        for (unsigned i = 0; i < dialogs; i++)
        {
            assert(fb.GetUint16LE() == 0);
            // Meaning of the below? Initial state of dialog?
            auto x = fb.GetUint8();
            auto y = fb.GetUint8();
            auto zero = fb.GetUint8();

            // Affects the dialog selected
            auto dialogKey = KeyTarget{fb.GetUint32LE()};
            std::cout << "#" << std::dec << i << std::hex << " " 
                << +x << " " << +y << " " << " dialogKey: " 
                << dialogKey << " target: " << dialogStore.GetTarget(dialogKey)
                << std::endl;

            const auto& emplaced = mKeys.emplace_back(dialogKey);
        }
    }

    const auto& GetKeys() const { return mKeys; }

private:
    std::vector<Target> mKeys;

    const ZoneLabel& mZoneLabel;
    const Logging::Logger& mLogger;
};

}
