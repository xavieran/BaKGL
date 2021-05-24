#include "src/dialog.hpp"

namespace BAK {

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
}

}
