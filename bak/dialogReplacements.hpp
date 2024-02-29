#pragma once

#include "bak/dialogAction.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/constants.hpp"

namespace BAK {

// Version 1.02 is rather buggy - many dialog actions seem to be
// scrambled - I have substituted these as appropriate from
// v1.01 dialog files.
struct Replacement
{
    OffsetTarget mTarget;
    std::vector<std::pair<unsigned, DialogAction>> mReplacements;
};

class Replacements
{
public:
    static void ReplaceActions(OffsetTarget target, std::vector<DialogAction>& actions);

private:
    static const std::vector<Replacement> sReplacements;
};

}
