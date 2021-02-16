#pragma once

#include "constants.hpp"
#include "logger.hpp"

#include "Exception.h"
#include "FileBuffer.h"

#include <boost/functional/hash.hpp>

#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <map>
#include <variant>

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
            std::cout << "I: " << i << " " << std::hex << fb.GetUint16LE() << std::dec << std::endl;
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
    Chapter = 0x7537,

};

struct KeyTarget
{
    std::uint32_t value;
};

struct OffsetTarget
{
	std::uint8_t dialogFile;
    std::uint32_t value;
};

using Target = std::variant<KeyTarget, OffsetTarget>;

std::ostream& operator<<(std::ostream& os, const Target& t)
{
	std::visit([&os](auto&& arg){
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, KeyTarget>)
			os << "Key [ " << std::hex << arg.value << " ]";
		else
			os << "Offset { " << std::hex << +arg.dialogFile << " @ " << arg.value << " }";
	}, t);
	return os;
}

class DialogSnippet
{
public:
    DialogSnippet(FileBuffer& fb, std::uint8_t dialogFile)
    {
        mDisplayStyle = fb.GetUint8();
        mActor = fb.GetUint16LE();
        mDisplayStyle2 = fb.GetUint8();
        mDisplayStyle3 = fb.GetUint8();
        std::uint8_t choices = fb.GetUint8();
        std::uint8_t actions = fb.GetUint8();
        auto length = fb.GetUint16LE();
        
        std::uint8_t i;

		const auto GetTarget = [&fb, dialogFile](auto targetRaw) -> Target
		{
			if (0x80000000 & targetRaw)
				return KeyTarget{targetRaw & 0x7fffffff};
			else
				return OffsetTarget{dialogFile, targetRaw};
		};

        for (i = 0; i < choices; i++)
        {
            auto state = fb.GetUint16LE();
            auto choice1 = fb.GetUint16LE();
            auto choice2 = fb.GetUint16LE();
            auto target = GetTarget(fb.GetUint32LE());
            mChoices.emplace_back(state, choice1, choice2, target);
        }

        for (i = 0; i < actions; i++)
        {
            mActions.emplace_back(fb.GetArray<10>());
        }
		
		if (length > 0)
			mText = fb.GetString(length);
    }

    struct Choice
    {
        std::uint16_t mState;
        std::uint16_t mChoice1;
        std::uint16_t mChoice2;
        Target mTarget;
    };

    struct Action
    {
        std::array<std::uint8_t, 10> mAction;
    };
    
    std::uint8_t mDisplayStyle;
    std::uint16_t mActor;
    std::uint8_t mDisplayStyle2;
    std::uint8_t mDisplayStyle3;

    std::vector<Choice> mChoices;
    std::vector<Action> mActions;
    std::string mText;
};

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& a)
{
	std::string sep = "";
	for (unsigned i = 0; i < N; i++)
	{
		os << sep << std::setw(2) << std::setfill('0') << +a[i];
		sep = " ";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const DialogSnippet& d)
{
    os << "[ ds: " << std::hex << +d.mDisplayStyle << " act: " << +d.mActor
        << " ds2: " << +d.mDisplayStyle2 << " ds3: " << +d.mDisplayStyle3 << " ]" << std::endl;
    
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

class Dialog
{
public:
    using KeyType = unsigned;

    Dialog()
    :
        mDialogMap{},
		mSnippetMap{},
        mLogger{Logging::LogState::GetLogger("Dialog")}
    {}

    void LoadKeys()
    {
        for (std::uint8_t dialogFile = 0; dialogFile < 32; dialogFile++)
        {
            auto fname = GetDialogFile(dialogFile);
            auto fb = FileBufferFactory::CreateFileBuffer(fname);
            unsigned dialogs = fb.GetUint16LE();
            mLogger.Info() << "Dialog " << fname << " has: " << dialogs << " dialogs" << std::endl;

            for (unsigned i = 0; i < dialogs; i++)
            {
                auto key = fb.GetUint32LE();
                auto val = fb.GetUint32LE();
                const auto [it, emplaced] = mDialogMap.emplace(
                    key,
                    std::make_pair(dialogFile, val));
                assert(emplaced);
                auto [checkF, checkV] = it->second;
                mLogger.Spam() << std::hex << "0x" << it->first 
                    << " -> 0x" << checkV << std::dec << std::endl;
            }

			while (fb.GetBytesLeft() > 0)
			{
				auto offset = fb.Tell();
				auto snippet = DialogSnippet{fb, dialogFile};
				mSnippetMap.emplace(
					std::make_pair(dialogFile, offset),
					snippet);
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
				std::cout << "Failed to walk dialog tree" << std::endl;
			}
		}
	}

    void ShowDialog(KeyType dialogKey)
    {
        auto it = mDialogMap.find(dialogKey);
        if (it == mDialogMap.end()) throw std::runtime_error("Key not found");
        auto [dialogFile, offset] = it->second;
        mLogger.Info() << "Loading dialog: 0x" << std::hex << dialogKey << " @ 0x"
            << offset << std::dec << " from: " << +dialogFile << std::endl;
		
		bool good = true;
		auto snippet = mSnippetMap.at(it->second);
		while (good)
		{
			std::cout << snippet << std::endl;
			if (snippet.mChoices.size() == 0)
			{
				std::cout << "Done" << std::endl;
				good = false;
			}
			else if (snippet.mChoices.size() == 1)
			{
				snippet = std::visit(*this, snippet.mChoices.back().mTarget);
			}
			else
			{
				snippet = std::visit(*this, snippet.mChoices.front().mTarget);
			}
		}
    }

	const DialogSnippet& operator()(KeyTarget dialogKey)
	{
		auto it = mDialogMap.find(dialogKey.value);
        if (it == mDialogMap.end()) throw std::runtime_error("Key not found");
		return (*this)(OffsetTarget{
			std::get<std::uint8_t>(it->second),
			std::get<unsigned>(it->second)});
	}

	const DialogSnippet& operator()(OffsetTarget snippetKey)
	{
		auto snip = mSnippetMap.find(
			std::make_pair(snippetKey.dialogFile, snippetKey.value));
		if (snip == mSnippetMap.end()) throw std::runtime_error("Offset not found");
		return snip->second;
	}

    std::string GetDialogFile(std::uint8_t i)
    {
        std::stringstream ss{};
        ss << "DIAL_Z" << std::setw(2) << std::setfill('0') << +i << ".DDX";
        return ss.str();
    }


private:
    using ValueType  = std::pair<std::uint8_t, unsigned>;
    using SnippetKey = std::pair<std::uint8_t, unsigned>;

	//std::unordered_map<KeyType, ValueType> mDialogMap;
	std::map<KeyType, ValueType> mDialogMap;
	std::unordered_map<
		SnippetKey,
		DialogSnippet,
		boost::hash<SnippetKey>> mSnippetMap;

    const Logging::Logger& mLogger;
};

}
