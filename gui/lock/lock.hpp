#pragma once

#include "bak/lock.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {
class Icons;

class Lock :
    public Widget
{
public:
    Lock(const Icons& icons, glm::vec2 pos);
    
    void AdjustShacklePosition(glm::vec2 diff);
    void SetLocked();
    void SetUnlocked();
    void SetImageBasedOnLockType(BAK::LockType lock);

private:
    void AddChildren();

private:
    const Icons& mIcons;

    Widget mShackle;
    Widget mLock;
};

}
