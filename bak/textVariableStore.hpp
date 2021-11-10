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
        mLogger{Logging::LogState::GetLogger("BAK::TextVariableStore")}
    {}

    std::string MakeVariableName(unsigned variable)
    {
        std::stringstream ss{};
        ss << "@" << variable;
        return ss.str();
    }

    void Clear()
    {
        mTextVariables.clear();
    }

    // variable 3: party magician
    // variable 4: party leader
    // variable 5: party non-magician
    void SetTextVariable(unsigned variable, std::string value)
    {
        mTextVariables[MakeVariableName(variable)] = value;
    }

    std::string SubstituteVariables(const std::string& text) const
    {
        auto newText = text;
        for (const auto& [key, value] : mTextVariables)
        {
            newText = std::regex_replace(
                newText,
                std::regex{key},
                value);
        }
        return newText;
    }

private:
    std::unordered_map<std::string, std::string> mTextVariables;

    const Logging::Logger& mLogger;
};

}
