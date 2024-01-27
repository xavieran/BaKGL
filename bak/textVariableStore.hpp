#pragma once

#include "bak/dialog.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"
#include "bak/gameData.hpp"
#include "bak/money.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

#include <regex>

namespace BAK {

class TextVariableStore
{
public:
    TextVariableStore()
    :
        mTextVariables{},
        mSelectedCharacter{},
        mLogger{Logging::LogState::GetLogger("BAK::TextVariableStore")}
    {}

    void Clear()
    {
        mTextVariables.clear();
    }

    void SetTextVariable(unsigned variable, std::string value)
    {
        mLogger.Spam() << "Setting " << variable << " to " << value << "\n";
        mTextVariables[MakeVariableName(variable)] = value;
    }

    void SetActiveCharacter(std::string value)
    {
        mSelectedCharacter = value;
    }

    std::string SubstituteVariables(const std::string& text) const
    {
        auto newText = text;

        for (const auto& [key, value] : mTextVariables)
        {
            mLogger.Spam() << "replacing " << key << " with " << value << "\n";
            newText = std::regex_replace(
                newText,
                std::regex{key},
                value);
        }

        // Do this last so it doesn't break all the others
        newText = std::regex_replace(
                newText,
                std::regex{"@"},
                mSelectedCharacter);


        return newText;
    }

private:
    std::string MakeVariableName(unsigned variable)
    {
        std::stringstream ss{};
        ss << "@" << variable;
        return ss.str();
    }

    std::unordered_map<std::string, std::string> mTextVariables;
    std::string mSelectedCharacter;

    const Logging::Logger& mLogger;
};

}
