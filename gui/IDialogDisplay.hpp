#pragma once


#include "bak/dialog.hpp"
#include "gui/IDialogScene.hpp"

namespace Gui {

class IDialogDisplay
{
    virtual void DisplayPlayer(IDialogScene& dialogScene, unsigned act) = 0;
    virtual std::pair<glm::vec2, std::string> DisplaySnippet(
        IDialogScene& dialogScene,
        const BAK::DialogSnippet& snippet,
        std::string_view remainingText)              = 0;
    virtual void ShowFlavourText(BAK::Target target) = 0;
    virtual void Clear()                             = 0;
};

}
