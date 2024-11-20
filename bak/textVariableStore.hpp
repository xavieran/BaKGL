#pragma once

#include "com/logger.hpp"

#include <string>
#include <unordered_map>

namespace BAK {

class TextVariableStore
{
public:
    TextVariableStore();

    void Clear();
    void SetTextVariable(unsigned variable, std::string value);
    void SetActiveCharacter(std::string value);
    std::string SubstituteVariables(const std::string& text) const;
private:
    std::string MakeVariableName(unsigned variable);

    std::unordered_map<std::string, std::string> mTextVariables;
    std::string mSelectedCharacter;

    const Logging::Logger& mLogger;
};

}
