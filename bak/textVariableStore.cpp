#include "bak/textVariableStore.hpp"

#include "com/logger.hpp"

#include <string>
#include <regex>
#include <unordered_map>

namespace BAK {

TextVariableStore::TextVariableStore()
:
    mTextVariables{},
    mSelectedCharacter{},
    mLogger{Logging::LogState::GetLogger("BAK::TextVariableStore")}
{}

void TextVariableStore::Clear()
{
    mTextVariables.clear();
}

void TextVariableStore::SetTextVariable(unsigned variable, std::string value)
{
    mLogger.Spam() << "Setting " << variable << " to " << value << "\n";
    mTextVariables.emplace(MakeVariableName(variable), value);
}

void TextVariableStore::SetActiveCharacter(std::string value)
{
    mSelectedCharacter = value;
}

std::string TextVariableStore::SubstituteVariables(const std::string& text) const
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

std::string TextVariableStore::MakeVariableName(unsigned variable)
{
    std::stringstream ss{};
    ss << "@" << variable;
    return ss.str();
}

}
